#include <QtTest>
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include <QSignalSpy>
#include <QTemporaryDir>

#include "backend.h"

class OmawriteTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase() {
        QVERIFY(m_settingsDirectory.isValid());
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                           m_settingsDirectory.path());
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
