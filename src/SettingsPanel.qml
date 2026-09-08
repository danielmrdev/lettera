import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Drawer {
    id: panel
    required property var preferences
    required property int activeLineHeight
    required property color pageColor
    required property color textColor
    objectName: "settingsPanel"
    edge: Qt.RightEdge
    width: Math.min(420, parent.width)
    height: parent.height
    modal: true
    focus: true
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    onOpened: fontPicker.forceActiveFocus()
    background: Rectangle { color: panel.pageColor }

    ScrollView {
        anchors.fill: parent
        anchors.margins: 24
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: parent.width
            spacing: 14
            RowLayout {
                Layout.fillWidth: true
                Label { text: "Settings"; font.pixelSize: 26; color: panel.textColor; Layout.fillWidth: true }
                ToolButton {
                    text: "Close"
                    Accessible.name: "Close settings"
                    onClicked: panel.close()
                }
            }
            Label { text: "Ctrl+, to toggle · Esc to close"; color: panel.textColor; opacity: 0.7 }
            Label { text: "Writing"; font.bold: true; color: panel.textColor }
            Label { text: "Font"; color: panel.textColor }
            ComboBox {
                id: fontPicker
                objectName: "fontPicker"
                Layout.fillWidth: true
                model: panel.preferences.fontFamilies
                currentIndex: model.indexOf(panel.preferences.fontFamily)
                Accessible.name: "Writing font"
                onActivated: panel.preferences.fontFamily = currentText
            }
            Label {
                text: "Font size · " + panel.preferences.fontSize + " px"
                color: panel.textColor
            }
            Slider {
                objectName: "fontSizeSlider"
                Layout.fillWidth: true
                from: 12; to: 40; stepSize: 1
                value: panel.preferences.fontSize
                Accessible.name: "Font size"
                onMoved: panel.preferences.fontSize = Math.round(value)
            }
            Label {
                text: "Writing width · " + panel.preferences.writingWidth + " px"
                color: panel.textColor
            }
            Slider {
                Layout.fillWidth: true
                from: 480; to: 1200; stepSize: 20
                value: panel.preferences.writingWidth
                Accessible.name: "Writing width"
                onMoved: panel.preferences.writingWidth = Math.round(value)
            }
            Label {
                text: "Line height · " + (panel.preferences.lineHeight / 100).toFixed(2) + "×"
                color: panel.textColor
            }
            Slider {
                Layout.fillWidth: true
                from: 100; to: 220; stepSize: 5
                value: panel.preferences.lineHeight
                Accessible.name: "Line height"
                onMoved: panel.preferences.lineHeight = Math.round(value)
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: panel.preferences.lineHeight !== panel.activeLineHeight
                    ? "Pending: applies when you next open a document or window. Current: "
                      + (panel.activeLineHeight / 100).toFixed(2) + "×."
                    : "Line height applies on document open, preserving undo history."
                color: panel.textColor
                opacity: 0.7
            }
            Label { text: "Appearance"; font.bold: true; color: panel.textColor }
            Switch {
                Layout.fillWidth: true
                text: "Follow system text scale"
                checked: panel.preferences.followSystemScale
                onToggled: panel.preferences.followSystemScale = checked
            }
            Switch {
                Layout.fillWidth: true
                text: "Show word count"
                checked: panel.preferences.showWordCount
                onToggled: panel.preferences.showWordCount = checked
            }
            Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: "Colours follow Omarchy. Autosave stays on."
                color: panel.textColor
                opacity: 0.7
            }
            Button {
                text: "Reset writing preferences"
                Layout.fillWidth: true
                onClicked: panel.preferences.reset()
            }
            Label { text: "Preferences saved automatically"; color: panel.textColor; opacity: 0.7 }
        }
    }
}
