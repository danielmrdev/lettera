#include "writingsettings.h"

#include <QFontDatabase>
#include <QSettings>

namespace {
QString defaultFont() { return QStringLiteral("iA Writer Mono S"); }

int boundedSetting(const QSettings &settings, const QString &key,
                   int fallback, int minimum, int maximum) {
    bool ok = false;
    const int value = settings.value(key, fallback).toInt(&ok);
    return ok ? qBound(minimum, value, maximum) : fallback;
}
}

WritingSettings::WritingSettings(QObject *parent) : QObject(parent) {
    QSettings settings;
    settings.beginGroup(QStringLiteral("writing"));
    m_fontFamily = settings.value(QStringLiteral("fontFamily"), defaultFont()).toString();
    if (!fontFamilies().contains(m_fontFamily))
        m_fontFamily = defaultFont();
    m_fontSize = boundedSetting(settings, QStringLiteral("fontSize"), 20, 12, 40);
    m_writingWidth = boundedSetting(settings, QStringLiteral("width"), 720, 480, 1200);
    m_lineHeight = boundedSetting(settings, QStringLiteral("lineHeight"), 140, 100, 220);
    m_followSystemScale = settings.value(QStringLiteral("followSystemScale"), true).toBool();
    m_showWordCount = settings.value(QStringLiteral("showWordCount"), false).toBool();
}

QStringList WritingSettings::fontFamilies() const {
    QStringList families = QFontDatabase::families();
    if (!families.contains(defaultFont()))
        families.prepend(defaultFont());
    return families;
}

void WritingSettings::persist() {
    QSettings settings;
    settings.beginGroup(QStringLiteral("writing"));
    settings.setValue(QStringLiteral("fontFamily"), m_fontFamily);
    settings.setValue(QStringLiteral("fontSize"), m_fontSize);
    settings.setValue(QStringLiteral("width"), m_writingWidth);
    settings.setValue(QStringLiteral("lineHeight"), m_lineHeight);
    settings.setValue(QStringLiteral("followSystemScale"), m_followSystemScale);
    settings.setValue(QStringLiteral("showWordCount"), m_showWordCount);
    emit changed();
}

void WritingSettings::setFontFamily(const QString &value) {
    if (value == m_fontFamily || !fontFamilies().contains(value))
        return;
    m_fontFamily = value;
    persist();
}

void WritingSettings::setFontSize(int value) {
    value = qBound(12, value, 40);
    if (value == m_fontSize) return;
    m_fontSize = value;
    persist();
}

void WritingSettings::setWritingWidth(int value) {
    value = qBound(480, value, 1200);
    if (value == m_writingWidth) return;
    m_writingWidth = value;
    persist();
}

void WritingSettings::setLineHeight(int value) {
    value = qBound(100, value, 220);
    if (value == m_lineHeight) return;
    m_lineHeight = value;
    persist();
}

void WritingSettings::setFollowSystemScale(bool value) {
    if (value == m_followSystemScale) return;
    m_followSystemScale = value;
    persist();
}

void WritingSettings::setShowWordCount(bool value) {
    if (value == m_showWordCount) return;
    m_showWordCount = value;
    persist();
}

void WritingSettings::reset() {
    m_fontFamily = defaultFont();
    m_fontSize = 20;
    m_writingWidth = 720;
    m_lineHeight = 140;
    m_followSystemScale = true;
    m_showWordCount = false;
    persist();
}
