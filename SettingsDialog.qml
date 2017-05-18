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

                        placeholderText: "\\\\%Server_name%\\FolderName или C:\\FolderName"
                        Layout.fillWidth: true
                        Component.onCompleted: {
                            text = MainClass.backupFolderName;
                            console.log("Backup folder " + text);
                        }
                        onTextChanged: {
                            width = hiddenText.contentWidth;
                        }

                        validator: RegExpValidator {
                            //regExp: /(\w:\\)|(\/\/)\w.*/
                            regExp: /(\w:\\)|(\\\\)|(\/\/)\w.*/
                        }

                    }
                    ToolButton {
                        id: chooseFilePath
                        height: backupFolderPath.height
                        Image {
                            source: Qt.resolvedUrl("qrc:/images/add.svg")
                            anchors.fill: parent
                            anchors.margins: 7
                        }
                        onClicked: {
                            loader.setSource("Filedialog.qml",{"context": backupFolderPath, "selectFolder":true})
                            //filedialog.selectFolder = true;
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

                        placeholderText: "3"
                        ToolTip.text: "Смещение относительно метки времени, присвоенной пакету на уровне ПЛК (часы)"

                        Layout.fillWidth: true
                        validator: RegExpValidator {
                            regExp: /[+-]?\d{0,2}/
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
                RowLayout {

                    Label {
                        text: "Путь до файла состояния резервирования:"
                    }
                    Text {
                        id: redundancyFilehidden
                        anchors.fill: redundancyFilePath
                        text: redundancyFilePath.text
                        visible: false
                    }
                    MyTextField {
                        id: redundancyFilePath

                        enabled: redundant.checked

                        ToolTip.text: "Путь до файла, по изменению которого отслеживается мастерство запущенных экземпляров приложения.
                                       \nФормат записи в файле:\n 0 - текущий сервер slave, 1 - master"

                        placeholderText: "\\\\%Server_name%\\FolderName\\file.txt или C:\\FolderName\\file.txt"
                        Layout.fillWidth: true
                        Component.onCompleted: {
                            text = MainClass.redundancyFilePath;
                            //console.log("Backup folder " + text);
                        }
                        onTextChanged: {
                            width = redundancyFilehidden.contentWidth;
                        }

                        validator: RegExpValidator {
                            //regExp: /(\w:\\)|(\/\/)\w.*/
                            regExp: /(\w:\\)|(\\\\)|(\/\/)\w.*/
                        }

                    }
                    ToolButton {
                        id: chooseRedFilePath
                        height: redundancyFilePath.height
                        visible: redundant.checked
                        Image {
                            source: Qt.resolvedUrl("qrc:/images/add.svg")
                            anchors.fill: parent
                            anchors.margins: 7
                        }
                        onClicked: {
                            //filedialog.selectFolder = false;
                            loader.setSource("Filedialog.qml",{"context": redundancyFilePath, "selectFolder":false})
                            loader.active = true
                        }
                    }

                    CheckBox {
                        id: redundant
                        text: "Резервир."
                        indicator.height: redundancyFilePath.height
                        indicator.width: indicator.height
                        //anchors.margins: 10
                        Component.onCompleted: {
                            checked = MainClass.redundant
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

                        placeholderText: "\\\\%Server_name%\\[FastLoadPath]"
                        Layout.fillWidth: true
                        Component.onCompleted: {
                            text = MainClass.serverName
                        }
                    }
                }
                RowLayout {
                    //anchors.fill: parent
                    Label {
                        text: "Коэффициент времени накопления одного сегмента данных"
                    }
                    MyTextField {
                        id: segmentInterval

                        ToolTip.text: "Параметр помогает учесть нагрузку сети и сервера historian.\n
Параметр определяет размеры .csv файлов импорта и частоту их копирования на сервер.\n
Интервал отправки данных на сервер можно рассчитать: период отпр. на сервер = коэф.* 10 * период обмена с ПЛК"

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
       //sourceComponent: filedialog
    }
/*
    Filedialog {
        id: filedialog
    }
*/
    onAccepted: {
        console.log("in SettingsDialog.qml - Settings Accepted start!");

        MainClass.resetError();

        MainClass.setServerName(historianName.text);
        MainClass.setAutostart(autoStart.checked);
        MainClass.setBackupFolderName(backupFolderPath.text);
        MainClass.setRedundant(redundant.checked);
        MainClass.setRedundancyFilePath(redundancyFilePath.text);
        MainClass.setTimeZone(timeZone.text);
        MainClass.setSegmentInterval(segmentInterval.text);

        console.log("in SettingsDialog.qml - Settings Accepted finished!");
    }

    Component.onCompleted: {
        width = grid.implicitWidth + 2 * grid.rowSpacing;
        height = grid.implicitHeight + 2 * grid.columnSpacing;
    }
}

