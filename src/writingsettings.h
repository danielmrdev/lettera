#pragma once

#include <QObject>
#include <QStringList>

// Presentation preferences only: never stored in the text document.
class WritingSettings : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString fontFamily READ fontFamily WRITE setFontFamily NOTIFY changed)
    Q_PROPERTY(int fontSize READ fontSize WRITE setFontSize NOTIFY changed)
    Q_PROPERTY(int writingWidth READ writingWidth WRITE setWritingWidth NOTIFY changed)
    Q_PROPERTY(int lineHeight READ lineHeight WRITE setLineHeight NOTIFY changed)
    Q_PROPERTY(bool followSystemScale READ followSystemScale WRITE setFollowSystemScale NOTIFY changed)
    Q_PROPERTY(bool showWordCount READ showWordCount WRITE setShowWordCount NOTIFY changed)
    Q_PROPERTY(QStringList fontFamilies READ fontFamilies CONSTANT)

public:
    explicit WritingSettings(QObject *parent = nullptr);
    QString fontFamily() const { return m_fontFamily; }
    int fontSize() const { return m_fontSize; }
    int writingWidth() const { return m_writingWidth; }
    int lineHeight() const { return m_lineHeight; }
    bool followSystemScale() const { return m_followSystemScale; }
    bool showWordCount() const { return m_showWordCount; }
    QStringList fontFamilies() const;

    void setFontFamily(const QString &value);
    void setFontSize(int value);
    void setWritingWidth(int value);
    void setLineHeight(int value);
    void setFollowSystemScale(bool value);
    void setShowWordCount(bool value);
    Q_INVOKABLE void reset();

signals:
    void changed();

private:
    void persist();
    QString m_fontFamily;
    int m_fontSize = 20;
    int m_writingWidth = 720;
    int m_lineHeight = 140;
    bool m_followSystemScale = true;
    bool m_showWordCount = false;
};
