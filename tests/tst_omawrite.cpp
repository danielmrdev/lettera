#include <QtTest>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QQuickItem>
#include <QQuickTextDocument>
#include <QQuickStyle>
#include <QFontDatabase>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>
#include <QStandardPaths>

#include "backend.h"

class OmawriteTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        QStandardPaths::setTestModeEnabled(true);
        QCoreApplication::setApplicationName(QStringLiteral("omawrite-settings-tests"));
        QQuickStyle::setStyle(QStringLiteral("Material"));
        QFontDatabase::addApplicationFont(QStringLiteral(":/fonts/iAWriterMonoS-Regular.ttf"));
        QVERIFY(m_settingsDirectory.isValid());
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                           m_settingsDirectory.path());
    }

    void init() {
        QSettings().clear();
    }

    void preferencesPersistAndResetOnlyWritingSettings() {
        QSettings().setValue(QStringLiteral("file/lastOpenDirectory"), QStringLiteral("/tmp/keep"));
        WritingSettings settings;
        QCOMPARE(settings.fontSize(), 20);
        QCOMPARE(settings.writingWidth(), 720);
        QCOMPARE(settings.lineHeight(), 140);
        QVERIFY(settings.followSystemScale());
        QVERIFY(!settings.showWordCount());
        const QString family = settings.fontFamilies().last();
        settings.setFontFamily(family);
        settings.setFontSize(28);
        settings.setWritingWidth(1000);
        settings.setLineHeight(180);
        settings.setFollowSystemScale(false);
        settings.setShowWordCount(true);
        WritingSettings restored;
        QCOMPARE(restored.fontFamily(), family);
        QCOMPARE(restored.fontSize(), 28);
        QCOMPARE(restored.writingWidth(), 1000);
        QCOMPARE(restored.lineHeight(), 180);
        QVERIFY(!restored.followSystemScale());
        QVERIFY(restored.showWordCount());
        restored.reset();
        WritingSettings defaults;
        QCOMPARE(defaults.fontFamily(), QStringLiteral("iA Writer Mono S"));
        QCOMPARE(defaults.fontSize(), 20);
        QCOMPARE(defaults.writingWidth(), 720);
        QCOMPARE(defaults.lineHeight(), 140);
        QVERIFY(defaults.followSystemScale());
        QVERIFY(!defaults.showWordCount());
        QCOMPARE(QSettings().value(QStringLiteral("file/lastOpenDirectory")).toString(),
                 QStringLiteral("/tmp/keep"));
    }

    void invalidPreferencesAreBounded() {
        QSettings().setValue(QStringLiteral("writing/fontFamily"), QStringLiteral("missing-font-xyz"));
        QSettings().setValue(QStringLiteral("writing/fontSize"), QStringLiteral("invalid"));
        QSettings().setValue(QStringLiteral("writing/width"), -9);
        QSettings().setValue(QStringLiteral("writing/lineHeight"), 999);
        WritingSettings settings;
        QCOMPARE(settings.fontFamily(), QStringLiteral("iA Writer Mono S"));
        QCOMPARE(settings.fontSize(), 20);
        QCOMPARE(settings.writingWidth(), 480);
        QCOMPARE(settings.lineHeight(), 220);
        settings.setFontSize(99);
        settings.setWritingWidth(9999);
        settings.setLineHeight(-1);
        settings.setFontFamily(QStringLiteral("missing-font-xyz"));
        QCOMPARE(settings.fontSize(), 40);
        QCOMPARE(settings.writingWidth(), 1200);
        QCOMPARE(settings.lineHeight(), 100);
        QCOMPARE(settings.fontFamily(), QStringLiteral("iA Writer Mono S"));
    }

    void settingsPanelPreservesTextHistoryAndAutosave() {
        QTemporaryDir directory;
        const QString path = directory.filePath(QStringLiteral("draft.txt"));
        QFile file(path);
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("First line\nSecond line");
        file.close();
        Backend backend;
        QQmlApplicationEngine engine;
        QSignalSpy warnings(&engine, &QQmlApplicationEngine::warnings);
        engine.rootContext()->setContextProperty(QStringLiteral("backend"), &backend);
        engine.load(QUrl(QStringLiteral("qrc:/Main.qml")));
        QVERIFY(!engine.rootObjects().isEmpty());
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        QVERIFY(window);
        auto *editor = window->findChild<QQuickItem *>(QStringLiteral("editor"));
        auto *panel = window->findChild<QObject *>(QStringLiteral("settingsPanel"));
        QVERIFY(editor);
        QVERIFY(panel);
        backend.open(QUrl::fromLocalFile(path));
        auto *quickDocument = qvariant_cast<QQuickTextDocument *>(editor->property("textDocument"));
        QVERIFY(quickDocument);
        QTextDocument *document = quickDocument->textDocument();
        QCOMPARE(document->begin().blockFormat().lineHeight(), 140.0);
        window->requestActivate();
        QTRY_VERIFY(window->isActive());
        editor->forceActiveFocus();
        QTest::keyClick(window, Qt::Key_End, Qt::ControlModifier);
        for (char key : QByteArray(" edited"))
            QTest::keyClick(window, key);
        const QString edited = document->toPlainText();
        QVERIFY(edited.endsWith(QStringLiteral(" edited")));
        QTRY_VERIFY_WITH_TIMEOUT(!backend.modified(), 3000);
        QSignalSpy saves(&backend, &Backend::saveSucceeded);
        document->undo();
        QVERIFY(document->isRedoAvailable());
        const int undoSteps = document->availableUndoSteps();
        const int redoSteps = document->availableRedoSteps();
        auto *settings = backend.writingSettings();
        settings->setFontSize(28);
        settings->setWritingWidth(1000);
        settings->setLineHeight(180);
        settings->setFontFamily(settings->fontFamilies().last());
        settings->setFollowSystemScale(false);
        settings->setShowWordCount(true);
        QCOMPARE(document->availableUndoSteps(), undoSteps);
        QCOMPARE(document->availableRedoSteps(), redoSteps);
        QCOMPARE(backend.activeLineHeight(), 140);
        QCOMPARE(document->begin().blockFormat().lineHeight(), 140.0);
        document->redo();
        QCOMPARE(document->toPlainText(), edited);
        QTRY_VERIFY_WITH_TIMEOUT(!backend.modified(), 3000);
        saves.clear();
        const QByteArray savedText = edited.toUtf8();
        QVERIFY(file.open(QIODevice::ReadOnly));
        QCOMPARE(file.readAll(), savedText);
        file.close();

        QTest::keyClick(window, Qt::Key_Comma, Qt::ControlModifier);
        QTRY_VERIFY(panel->property("opened").toBool());
        const QString screenshot = qEnvironmentVariable("OMAWRITE_TEST_SCREENSHOT");
        if (!screenshot.isEmpty()) {
            QTest::qWait(200);
            QVERIFY(window->grabWindow().save(screenshot));
        }
        auto *sizeSlider = window->findChild<QQuickItem *>(QStringLiteral("fontSizeSlider"));
        QVERIFY(sizeSlider);
        sizeSlider->forceActiveFocus();
        QTest::keyClick(window, Qt::Key_Right);
        QCOMPARE(settings->fontSize(), 29);
        QTest::keyClick(window, Qt::Key_Escape);
        QTRY_VERIFY(!panel->property("visible").toBool());
        QTRY_VERIFY(editor->hasActiveFocus());
        QTest::keyClick(window, Qt::Key_Comma, Qt::ControlModifier);
        QTRY_VERIFY(panel->property("opened").toBool());
        QTest::keyClick(window, Qt::Key_Comma, Qt::ControlModifier);
        QTRY_VERIFY(!panel->property("visible").toBool());
        window->resize(640, 420);
        QTest::keyClick(window, Qt::Key_Comma, Qt::ControlModifier);
        QTRY_VERIFY(panel->property("opened").toBool());
        QTest::keyClick(window, Qt::Key_Escape);
        QTRY_VERIFY(!panel->property("visible").toBool());
        QVERIFY(editor->width() <= window->width() - 72);
        settings->setFontSize(30);
        backend.setTextScale(1.5);
        QCOMPARE(editor->property("font").value<QFont>().pixelSize(), 30);
        settings->setFollowSystemScale(true);
        QCOMPARE(editor->property("font").value<QFont>().pixelSize(), 45);
        QTest::qWait(1100);
        QCOMPARE(saves.count(), 0);
        QVERIFY(!backend.modified());
        QCOMPARE(document->toPlainText(), edited);

        // Newly inserted paragraphs still use the active, not pending, spacing.
        editor->forceActiveFocus();
        QTest::keyClick(window, Qt::Key_End, Qt::ControlModifier);
        QTest::keyClick(window, Qt::Key_Return);
        for (char key : QByteArray("third"))
            QTest::keyClick(window, key);
        QCOMPARE(document->lastBlock().blockFormat().lineHeight(), 140.0);
        QTRY_VERIFY_WITH_TIMEOUT(!backend.modified(), 3000);
        backend.open(QUrl::fromLocalFile(path));
        QCOMPARE(backend.activeLineHeight(), 180);
        QCOMPARE(document->begin().blockFormat().lineHeight(), 180.0);
        QCOMPARE(document->lastBlock().blockFormat().lineHeight(), 180.0);

        const QString longPath = directory.filePath(QStringLiteral("long.txt"));
        QFile longFile(longPath);
        QVERIFY(longFile.open(QIODevice::WriteOnly));
        longFile.write(QByteArray("A long document needs vertical scrolling.\n").repeated(100));
        longFile.close();
        backend.open(QUrl::fromLocalFile(longPath));
        editor->forceActiveFocus();
        QTest::keyClick(window, Qt::Key_End, Qt::ControlModifier);
        QTRY_VERIFY(editor->height() > window->height());
        const auto cursorBottom = [editor]() {
            return editor->mapToScene(editor->property("cursorRectangle").toRectF().bottomLeft()).y();
        };
        QTRY_VERIFY(cursorBottom() > 42);
        QTRY_VERIFY(cursorBottom() <= window->height() - 42);
        QCOMPARE(warnings.count(), 0);
    }

    void countsWords() {
        QCOMPARE(Backend::countWords(QStringLiteral("one two-three don't 42")), 4);
        QCOMPARE(Backend::countWords(QStringLiteral("你好 世界")), 2);
        QCOMPARE(Backend::countWords(QString()), 0);
    }

    void suggestsTxtNamesFromFirstLine() {
        QCOMPARE(Backend::suggestedFileName(QStringLiteral("My first draft\nBody")),
                 QStringLiteral("My first draft.txt"));
        QCOMPARE(Backend::suggestedFileName(QStringLiteral("A/B")),
                 QStringLiteral("A-B.txt"));
        QCOMPARE(Backend::suggestedFileName(QString()), QStringLiteral("Untitled.txt"));
        QCOMPARE(Backend::suggestedFileName(QStringLiteral("Already.txt")),
                 QStringLiteral("Already.txt"));
    }

    void loadsCurrentOmarchyTheme() {
        QTemporaryDir homeDirectory;
        QVERIFY(homeDirectory.isValid());
        const QByteArray originalHome = qgetenv("HOME");
        struct HomeRestorer { QByteArray value; ~HomeRestorer() { qputenv("HOME", value); } }
            restoreHome{originalHome};
        QVERIFY(qputenv("HOME", homeDirectory.path().toUtf8()));

        const QString themeDirectory = homeDirectory.path()
            + QStringLiteral("/.local/state/omarchy/current/theme");
        QVERIFY(QDir().mkpath(themeDirectory));
        QFile colorsFile(themeDirectory + QStringLiteral("/colors.toml"));
        QVERIFY(colorsFile.open(QIODevice::WriteOnly | QIODevice::Text));
        const QByteArray palette(
            "mode = \"light\"\n"
            "accent = \"#112233\"\n"
            "selection = \"#445566\"\n"
            "background = \"#fefefe\"\n"
            "foreground = \"#101010\"\n");
        QCOMPARE(colorsFile.write(palette), qint64(palette.size()));
        colorsFile.close();

        Backend backend;
        QCOMPARE(backend.themeBackground(), QStringLiteral("#fefefe"));
        QCOMPARE(backend.themeForeground(), QStringLiteral("#101010"));
        QVERIFY(!backend.darkMode());
    }

    void remembersLastOpenDirectoryAndUsesAtomicTxtSave() {
        QTemporaryDir directory;
        QVERIFY(directory.isValid());
        const QString path = directory.filePath(QStringLiteral("first.txt"));
        QFile initial(path);
        QVERIFY(initial.open(QIODevice::WriteOnly));
        QVERIFY(initial.write("hello"));
        initial.close();

        Backend backend;
        backend.open(QUrl::fromLocalFile(path));
        QCOMPARE(backend.lastOpenDirectory().toLocalFile(), directory.path());
        QCOMPARE(backend.fileName(), QStringLiteral("first.txt"));

        QSignalSpy externalChangeSpy(&backend, &Backend::externalChangeDetected);
        QFile sameContents(path);
        QVERIFY(sameContents.open(QIODevice::WriteOnly | QIODevice::Truncate));
        sameContents.write("hello");
        sameContents.close();
        QTest::qWait(100);
        QCOMPARE(externalChangeSpy.count(), 0);
    }

    void qmlHasOnlyWritingSurface() {
        const QString qmlPath = QFileInfo(QDir::currentPath()
            + QStringLiteral("/../src/Main.qml")).absoluteFilePath();
        QFile sourceFile(qmlPath);
        QVERIFY(sourceFile.open(QIODevice::ReadOnly));
        const QByteArray source = sourceFile.readAll();
        QVERIFY(source.contains("TextArea"));
        QVERIFY(source.contains("Text files (*.txt)"));
        QVERIFY(!source.contains("Markdown"));
        QVERIFY(!source.contains("FooterIconButton"));
    }

private:
    QTemporaryDir m_settingsDirectory;
};

QTEST_MAIN(OmawriteTest)
#include "tst_omawrite.moc"
