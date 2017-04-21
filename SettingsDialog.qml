import QtQuick 2.1
import QtQuick.Controls 2.1
import QtQuick.Dialogs 1.2 as AppDialogs
import QtQuick.Layouts 1.1

AppDialogs.Dialog {
    id: dialog
    modality: Qt.WindowModal
    title: qsTr("Настройки")
    //width: grid.implicitWidth + 2 * grid.rowSpacing
    //height: grid.implicitHeight + 2 * grid.columnSpacing

    //standardButtons: StandardButton.Save | StandardButton.Discard

    signal finished(string fullName, string address, string city, string number)

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
                    //anchors.fill: parent
                    Label {
                        text: "Каталог временных файлов:"
                    }

                    Text {
                        id: hiddenText
                        anchors.fill: backupFolderPath
                        text: backupFolderPath.text
                        visible: false
                    }

                    MyTextField {
                        id: backupFolderPath

                        ToolTip.text: "Каталог для сформированных .csv файлов.\nЕсли запись идет напрямую\n в БД, должен быть на Historian, иначе на локальной машине"

                        placeholderText: "//A_Server/FastRetro или C:\\FastRetro"
                        Layout.fillWidth: true
                        Component.onCompleted: {
                            text = MainClass.backupFolderName;
                            console.log("Backup folder " + text);
                        }
                        onTextChanged: {
                            width = hiddenText.contentWidth;
                        }

                        validator: RegExpValidator {
                            regExp: /(\w:\\)|(\/\/)\w.*/
                        }

                    }
                    ToolButton {
                        id: chooseFilePath
                        height: backupFolderPath.height
                        Image {
                            source: Qt.resolvedUrl("qrc:/images/add.svg")
                            anchors.fill: parent
                            anchors.margins: 4
                        }
                        onClicked: {
                            loader.active = true
                        }
                    }
                }

                RowLayout {

                    Label {
                        text: "Часовой пояс сервера относительно ПЛК:"
                    }
                    MyTextField {
                        id: timeZone

                        placeholderText: "+3"
                        Layout.fillWidth: true
                        validator: RegExpValidator {
                            regExp: /[+-]\d{0,2}/
                        }
                        Component.onCompleted: {
                            text = MainClass.timeZone
                        }

                    }

                    CheckBox {
                        id: autoStart
                        text: "Автостарт"
                        indicator.height: timeZone.height
                        indicator.width: indicator.height
                        //anchors.margins: 10
                        Component.onCompleted: {
                            checked = MainClass.autostart
                            height = parent.height
                        }
                    }
                }

            }

        }

        GroupBox {
            title: "Архивирование"

            Layout.fillWidth: true
            Layout.columnSpan: grid.columns
            ColumnLayout {
                Layout.fillWidth: true
                anchors.fill: parent
                RowLayout {
                    //anchors.fill: parent
                    Label {
                        text: "Сетевой путь до Historian"
                    }
                    MyTextField {
                        id: historianName

                        ToolTip.text: "Укажите сетевое имя сервера Historian. Если предполагается вставка данных\n
                                   родными средствами Wonderware, укажите дополнительно путь до каталога FastLoad"

                        validator: RegExpValidator {
                            regExp: /\\\\\w.*(\\.*)?/
                        }

                        placeholderText: "\\%Server_name%\[FastLoadPath]"
                        Layout.fillWidth: true
                        Component.onCompleted: {
                            text = MainClass.serverName
                        }
                    }
                }
                RowLayout {
                    //anchors.fill: parent
                    Label {
                        text: "Время накопления одного сегмента данных в сек."
                    }
                    MyTextField {
                        id: segmentInterval

                        ToolTip.text: "Параметр помогает учесть нагрузку сети и сервера historian.\n
                                       Параметр определяет размеры .csv файлов импорта и частоту их копирования на сервер"

                        validator: RegExpValidator {
                            regExp: /([1]\d)|[1-9]/
                        }

                        placeholderText: "1..19"
                        Layout.fillWidth: true
                        Component.onCompleted: {
                            text = MainClass.segmentInterval
                        }
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
                var path = url.match('file\:///(.*)');
                if(path !== null)
                    backupFolderPath.text = path[1];
                else
                    backupFolderPath.placeholderText = "Некорректный путь...";
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
        console.log("in SettingsDialog.qml - Settings Accepted start!");

        MainClass.resetError();

        MainClass.setServerName(historianName.text);
        MainClass.setAutostart(autoStart.checked);
        MainClass.setBackupFolderName(backupFolderPath.text);
        MainClass.setTimeZone(timeZone.text);
        MainClass.setSegmentInterval(segmentInterval.text);

        console.log("in SettingsDialog.qml - Settings Accepted finished!");
    }

    Component.onCompleted: {
        width = grid.implicitWidth + 2 * grid.rowSpacing;
        height = grid.implicitHeight + 2 * grid.columnSpacing;
    }
}

