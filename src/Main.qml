import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Dialogs as Dialogs
import QtQuick.Window

ApplicationWindow {
    id: win
    width: 1100
    height: 760
    minimumWidth: 640
    minimumHeight: 420
    visible: true
    title: backend.fileName
    color: backend.themeBackground

    readonly property bool darkMode: backend.darkMode
    readonly property real textScale: backend.textScale
    readonly property color pageColor: backend.themeBackground
    readonly property color textColor: backend.themeForeground
    readonly property color selectionColor: backend.themeSelection
    readonly property var preferences: backend.writingSettings
    readonly property int editorFontPixelSize: Math.max(12, Math.round(
        preferences.fontSize * (preferences.followSystemScale ? textScale : 1)))
    property bool closeConfirmed: false

    Material.theme: darkMode ? Material.Dark : Material.Light
    Material.accent: backend.themeAccent

    onClosing: function(close) {
        if (closeConfirmed || !backend.modified || !backend.fileUrl.toString())
            return;
        close.accepted = false;
        backend.saveForClose();
    }

    Shortcut {
        sequence: "Ctrl+,"
        context: Qt.ApplicationShortcut
        onActivated: settingsPanel.visible ? settingsPanel.close() : settingsPanel.open()
    }

    SettingsPanel {
        id: settingsPanel
        preferences: win.preferences
        activeLineHeight: backend.activeLineHeight
        pageColor: win.pageColor
        textColor: win.textColor
        onClosed: editor.forceActiveFocus()
    }

    Shortcut {
        sequence: "Ctrl+O"
        enabled: !settingsPanel.visible
        context: Qt.ApplicationShortcut
        onActivated: backend.openDialog()
    }

    Shortcut {
        sequence: "Ctrl+S"
        enabled: !settingsPanel.visible
        context: Qt.ApplicationShortcut
        onActivated: backend.save()
    }

    Shortcut {
        sequence: "Ctrl+Shift+S"
        enabled: !settingsPanel.visible
        context: Qt.ApplicationShortcut
        onActivated: backend.saveAsDialog()
    }

    Shortcut {
        sequence: "Ctrl+Z"
        enabled: !settingsPanel.visible
        context: Qt.WindowShortcut
        onActivated: editor.undo()
    }

    Shortcut {
        sequences: ["Ctrl+Shift+Z", "Ctrl+Y"]
        enabled: !settingsPanel.visible
        context: Qt.WindowShortcut
        onActivated: editor.redo()
    }

    Connections {
        target: backend

        function onOpenDialogRequested() {
            openFileDialog.currentFolder = backend.lastOpenDirectory;
            openFileDialog.open();
        }

        function onSaveDialogRequested(suggestedUrl) {
            saveFileDialog.selectedFile = suggestedUrl;
            saveFileDialog.open();
        }

        function onCloseAfterSave() {
            win.closeConfirmed = true;
            win.close();
        }
    }

    Dialogs.FileDialog {
        id: openFileDialog
        title: "Open text file"
        fileMode: Dialogs.FileDialog.OpenFile
        nameFilters: ["Text files (*.txt)"]
        onAccepted: backend.open(selectedFile)
    }

    Dialogs.FileDialog {
        id: saveFileDialog
        title: "Save text file"
        fileMode: Dialogs.FileDialog.SaveFile
        nameFilters: ["Text files (*.txt)"]
        onAccepted: backend.saveAs(selectedFile)
        onRejected: backend.fileDialogCanceled()
    }

    ScrollView {
        id: writingView
        anchors.fill: parent
        anchors.leftMargin: Math.max(36, (win.width - preferences.writingWidth) / 2)
        anchors.rightMargin: anchors.leftMargin
        anchors.topMargin: 42
        anchors.bottomMargin: 42
        clip: true
        contentWidth: availableWidth

        TextArea {
            id: editor
            objectName: "editor"
            textFormat: TextEdit.PlainText
            text: ""
            color: win.textColor
            selectionColor: win.selectionColor
            selectedTextColor: win.pageColor
            font.family: win.preferences.fontFamily
            font.pixelSize: win.editorFontPixelSize
            wrapMode: TextEdit.Wrap
            selectByMouse: true
            persistentSelection: true
            tabStopDistance: font.pixelSize * 4
            background: Rectangle { color: "transparent" }

            placeholderText: "Start writing"
            placeholderTextColor: backend.themeForeground
            opacity: 0.98

            onTextChanged: backend.editorTextChanged()

            Component.onCompleted: {
                backend.attachDocument(textDocument);
                forceActiveFocus();
            }
        }
    }

    Label {
        objectName: "wordCount"
        visible: preferences.showWordCount
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 12
        anchors.horizontalCenter: parent.horizontalCenter
        text: backend.wordCount + (backend.wordCount === 1 ? " word" : " words")
        color: win.textColor
        opacity: 0.65
    }

    Component.onCompleted: {
        const geometry = backend.windowGeometry();
        if (geometry.x >= 0) x = geometry.x;
        if (geometry.y >= 0) y = geometry.y;
        width = geometry.width;
        height = geometry.height;
        if (geometry.maximized) showMaximized();
    }

    Component.onDestruction: backend.saveWindowGeometry(
        x, y, width, height, visibility === Window.Maximized)
}
