import QtQuick 2.1
import QtQuick.Controls 1.4
import QtQuick.Dialogs 1.2 as AppDialogs
import QtQuick.Layouts 1.1

AppDialogs.Dialog {
    id: dialog
    modality: Qt.WindowModal
    title: qsTr("Настройки")
    width: grid.implicitWidth + 2 * grid.rowSpacing
    height: grid.implicitHeight + 2 * grid.columnSpacing

    standardButtons: StandardButton.Save | StandardButton.Discard

    signal finished(string fullName, string address, string city, string number)
    property alias textColor : historianName.textColor

    GridLayout {
        id: grid

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: grid.rowSpacing
        anchors.rightMargin: grid.rowSpacing
        anchors.topMargin: grid.columnSpacing

        //columns: page.width < page.height ? 1 : 2

        columns: 1

        GroupBox {
            title: "Общие"

            Layout.fillWidth: true
            Layout.columnSpan: grid.columns

            ColumnLayout {
                Layout.fillWidth: true
                anchors.fill: parent
                //preferredWidth: rowLay.width

                RowLayout {
                    id: rowLay
                    anchors.fill: parent
                    Label {
                        text: "Каталог временных файлов:"
                    }
                    Text {
                        id: hiddenText
                        anchors.fill: backupFolder
                        text: backupFolder.text
                        visible: false
                    }
                    MyTextField {
                        id: backupFolder

                        property var oldText
                        //readOnly: true
                        placeholderText: "C:/"
                        Layout.fillWidth: true
                        Component.onCompleted: {
                            text = MainClass.backupFolderName
                        }
                        onTextChanged: {
                            width = hiddenText.contentWidth
                        }

                        validator: RegExpValidator {
                            regExp: /\/\/\w.*/
                        }

                    }
                    ToolButton {
                        id: chooseFilePath
                        height: backupFolder.height
                        //iconSource: Qt.resolvedUrl("qrc:/images/add.svg")
                        Image {
                            source: Qt.resolvedUrl("qrc:/images/add.svg")
                            anchors.fill: parent
                            anchors.margins: 4
                        }
                        onClicked: {
                            loader.active = true
                        }
                    }
                    //Item { Layout.fillWidth: true }
                    //}
                }


                CheckBox {
                    id: autoStart
                    text: "Автостарт"

                    Component.onCompleted: {
                        checked = MainClass.autostart
                    }
                }
            }

        }
        GroupBox {
            title: "Архивирование"

            Layout.fillWidth: true
            Layout.columnSpan: grid.columns
            RowLayout {
                anchors.fill: parent
                Label {
                    text: "Имя сервера:"
                }
                MyTextField {
                    id: historianName
                    //enabled: MainClass.savePermit
                    //text: MainClass.serverName
                    validator: RegExpValidator {
                        regExp: /[^А-я]*/
                    }

                    placeholderText: "A_Server"
                    Layout.fillWidth: true
                    Component.onCompleted: {
                        text = MainClass.serverName
                    }
                }
            }
        }
    }

    Loader {
       id: loader
       visible: status == Loader.Ready
       active: false
       sourceComponent: filedialog
    }

    Component {
        id: filedialog
        AppDialogs.FileDialog {
            //id: dialog
            //options: FolderDialog.DontResolveSymlinks | FolderDialog.ReadOnly
            selectFolder: true
            title: "Выберите путь до каталога хранения временных файлов"
            //currentFolder: shortcuts.home

            onAccepted: {
                console.log("You chose: " + folder)
                var url = folder.toString();
                var path = url.match('file\:(//\\w.*)');
                if(path[1].length > 0)
                    backupFolder.text = path[1];
                loader.active = false;
            }
            onRejected: {
                console.log("Canceled")
                loader.active = false
            }
            Component.onCompleted: visible = true
        }
    }

    onAccepted: {
        console.log("in SettingsDialog.qml - Settings Accepted!");
        MainClass.resetError();
        MainClass.setServerName(historianName.text);
        MainClass.setAutostart(autoStart.checked);
        MainClass.setBackupFolderName(backupFolder.text);
    }

}

