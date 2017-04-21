import QtQuick 2.5
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.2
import QtQuick.Particles 2.0
import QtQuick.Dialogs 1.2

ApplicationWindow {
    id: window
    minimumWidth: 640
    minimumHeight: 500
    //width: window.width < 640 ? 640 : window.width
    //height: 480
    visible: true
    title: qsTr("FastRetro App")

    toolBar: ToolBar {
        RowLayout{
            anchors.fill: parent
            ToolButton {
                id: startButton
                enabled: MainClass.startPermit
                //text: "Старт"
                tooltip: "Старт"
                //implicitWidth: window.width / columnFactor
                anchors.left: parent.left
                Image {
                    source: Qt.resolvedUrl("qrc:/images/play_arrow.svg")
                    anchors.fill: parent
                    anchors.margins: 4
                }
                onClicked: MainClass.connectToServer()
            }
            ToolButton {
                id: stopButton
                enabled: MainClass.stopPermit
                //text: "Стоп"
                tooltip: "Останов"
                Image {
                    source: Qt.resolvedUrl("qrc:/images/stop.svg")
                    anchors.fill: parent
                    anchors.margins: 4
                }
                //implicitWidth: window.width / columnFactor
                onClicked: MainClass.stopScan()
            }
            ToolButton {
                id: saveButton
                enabled: MainClass.savePermit
                //text: "Сохранить"
                Image {
                    source: Qt.resolvedUrl("qrc:/images/file_download.svg")
                    anchors.fill: parent
                    anchors.margins: 4
                }
                tooltip: "Сохранение карты IP-адресов в реестре"
                //implicitWidth: window.width / columnFactor
                onClicked: MainClass.saveConfig()
            }

            Item { Layout.fillWidth: true }
            ToolButton {
                id: warningLogButton
                //enabled: MainClass.savePermit
                visible: false
                Image {
                    source: Qt.resolvedUrl("qrc:/images/warning.svg")
                    anchors.fill: parent
                    anchors.margins: 4
                }
                tooltip: "Есть предупреждения"
                onClicked: logErrorsView.open()
            }
            ToolButton {
                height: startButton.height
                //width: window.width / columnFactor
                //anchors.right: parent.right
                anchors.right: parent.right
                enabled: MainClass.savePermit
                //iconSource: Qt.resolvedUrl("qrc:/images/settings_applications.svg")
                Image {
                    source: Qt.resolvedUrl("qrc:/images/settings_applications.svg")
                    anchors.fill: parent
                    anchors.margins: 4
                }
                //onClicked: settingsDialog.open()
                onClicked: settingsDialog.open()
            }
        }
    }
    property int columnFactor: 5

    SettingsDialog {
        id: settingsDialog
        width: 600
        onWidthChanged: {
            //if (width < 600) width = 600;
        }
        //height: 500                  
    }
    LogView {
        id: logErrorsView
        width: 600
        height: 300
        onWidthChanged: {
            //if (width < 600) width = 600;
        }
        onHeightChanged: {
            //if (height < 300) height = 300;
        }
        onReset: {
            warningLogButton.visible = false;
        }
    }

/*
    Loader {
        id: loaderSettingsDialog
        visible: status == Loader.Ready
        active: false
        sourceComponent: settingsDialog
    }
    Component {
        id: settingsDialog
        SettingsDialog {
            Component.onCompleted: {
                console.log("Loader complete!");
                //visible = true;
                open();
            }
            onRejected: {
                console.log("Rejected!");
                loaderSettingsDialog.active = false;
            }
            onAccepted: {
                console.log("Accepted!");
                loaderSettingsDialog.active = false;
            }
        }
    }
*/
    TableView {
        id: tableView

        frameVisible: false
        sortIndicatorVisible: false

        anchors.fill: parent

        Layout.minimumWidth: 400
        Layout.minimumHeight: 240
        Layout.preferredWidth: 600
        Layout.preferredHeight: 400

        TableViewColumn {
            id: ipAddrColumn
            title: "IP адрес"
            role: "ipAddr"
            movable: false
            resizable: false
            width: tableView.viewport.width - statusColumn.width
        }

        TableViewColumn {
            id: statusColumn
            title: "Статус"
            role: "status"
            movable: false
            resizable: false
            width: tableView.viewport.width / 3
        }

        model: AppAddressTable

        itemDelegate: editableDelegate;

        /*
        ListModel {
            id: sourceModel
            ListElement {
                ipAddr: "172."
                status: "Herman Melville"
            }
            ListElement {
                ipAddr: "The Adventures of Tom Sawyer"
                status: "Mark Twain"
            }

        }
        */
        Connections {
//            target: tableView.model
//            onDataChanged: {
//                console.log("Changing data")
//            }
        }


        Component {
            id: editableDelegate
            Item {

                function setColor(data, color){
                    //if(data == "Подключение")
                    //    return "green"
                    if(data.indexOf('Ошибка',0) !== -1)
                        return "red"
                    else
                        return "green"
                }
                function changeState(data){
                    if(data.indexOf('Ошибка',0) !== -1)
                        state = "disconnect";
                    else if(data.indexOf('Подключ',0) !== -1)
                        state = "connecting";
                    else if(data === "")
                        state = "wrong";
                    else
                        state = "stopByUser";
                }

                Text {
                    //width: parent.width
                    anchors.margins: 4
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    elide: styleData.elideMode
                    text: styleData.value !== undefined ? styleData.value : ""
                    color : "black"
                    visible: !styleData.selected
                    onTextChanged: {
                        this.color = setColor(styleData.value,color)
                        changeState(styleData.value);
                    }

                }
                Image {
                    id: forceStopConnectIcon
                    visible: false
                    source: Qt.resolvedUrl("qrc:/images/stop.svg")
                    fillMode: Image.PreserveAspectFit
                    height: parent.height
                    width: height
                    anchors.right: forceReconnectIcon.left
                    anchors.margins: 1
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            //Принудительно выполняем коннект
                            MainClass.forceStopConnect(styleData.Row);
                        }
                    }
                }
                Image {
                    id: forceStartConnectIcon
                    visible: false
                    source: Qt.resolvedUrl("qrc:/images/play_arrow.svg")
                    fillMode: Image.PreserveAspectFit
                    height: parent.height
                    width: height
                    anchors.right: forceReconnectIcon.left
                    anchors.margins: 1
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            //Принудительно выполняем коннект
                            MainClass.forceStartConnect(styleData.Row);
                        }
                    }
                }
                Image {
                    id: forceReconnectIcon
                    visible: false
                    source: Qt.resolvedUrl("qrc:/images/restore.svg")
                    fillMode: Image.PreserveAspectFit
                    height: parent.height
                    width: height
                    anchors.right: parent.right
                    anchors.margins: 1
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            //Принудительно выполняем реконнект
                            console.log("click force");
                            MainClass.forceReconnect(styleData.Row);
                        }
                    }
                }

                Loader {
                    id: loaderEditor
                    anchors.fill: parent
                    anchors.margins: 4
                    Connections {
                        target: loaderEditor.item
                        onEditingFinished: {
                            AppAddressTable.ipChange(styleData.row,styleData.column, loaderEditor.item.text)
                            //historianName.forceActiveFocus();
                        }
                    }
                    sourceComponent: (styleData.selected) ? editor : null
                    Component {
                        id: editor
                        TextInput {
                            id: textinput
                            color: styleData.textColor
                            text: styleData.value
                            enabled: state === "stopByUser"
                            MouseArea {
                                id: mouseArea
                                //enabled: styleData.column === 0
                                anchors.fill: parent
                                hoverEnabled: true
                                enabled: MainClass.startPermit
                                onClicked: textinput.forceActiveFocus()
                            }
                        }
                    }
                }
                states: [
                    State {
                        name: "connecting"
                        PropertyChanges { target: forceStopConnectIcon; visible: styleData.column === 1 }
                        PropertyChanges { target: forceStartConnectIcon; visible: false }
                        PropertyChanges { target: forceReconnectIcon; visible: false }
                        //PropertyChanges { target: textinput; enabled: false }
                    },
                    State {
                        name: "disconnect"
                        PropertyChanges { target: forceStopConnectIcon; visible: false }
                        PropertyChanges { target: forceStartConnectIcon; visible: false }
                        PropertyChanges { target: forceReconnectIcon; visible: styleData.column === 1 }
                        //PropertyChanges { target: textinput; enabled: false}
                    },
                    State {
                        name: "stopByUser"
                        PropertyChanges { target: forceStopConnectIcon; visible: false }
                        PropertyChanges { target: forceStartConnectIcon; visible: styleData.column === 1 }
                        PropertyChanges { target: forceReconnectIcon; visible: false }
                        //PropertyChanges { target: textinput; enabled: styleData.column === 0}
                    },
                    State {
                        name: "wrong"
                        PropertyChanges { target: forceStopConnectIcon; visible: false }
                        PropertyChanges { target: forceStartConnectIcon; visible: false }
                        PropertyChanges { target: forceReconnectIcon; visible: false }
                        //PropertyChanges { target: textinput; enabled: false}
                    }
                ]
                /*Component.onCompleted: {
                    state = "stopByUser"
                }*/

            }
        }
    }
    statusBar: StatusBar {
        id: statusBar
        visible: statusData.text.length > 0
        //width: parent.width
        property var error: MainClass.currentError.secondItem

        RowLayout {
            anchors.fill: parent
            Label {
                id: statusData
                text: MainClass.currentError.secondItem
                onTextChanged: {
                    ;//settingsDialog.textColor = (MainClass.currentError.firstItem === 3) ? "red" : "black";
                }
            }
        }
        onErrorChanged: {
            console.log("QML:: error changed " + MainClass.currentError.firstItem)
            if(MainClass.currentError.firstItem !== 10) {
                warningLoad.active = true
                warningLogButton.visible = true
            }
            if(MainClass.currentError.firstItem === 5) logErrorsView.errorFile = true
        }

    }
    Loader {
        id: warningLoad
        visible: status == Loader.Ready
        active: false
        sourceComponent: warningWindow
    }

    Component {
        id: warningWindow
       Dialog {
            standardButtons: StandardButton.Ok
            //informativeText:
            //text:
            //detailedText:
            Text {
                id: warningText
                anchors.fill: parent
                visible: parent.visible
            }

            onAccepted: warningLoad.active = false
            Component.onCompleted: {
                var states = ['','Сокет','Конфигурация','Ошибка подключения к БД','Системная ошибка','Ошибка лога'];
                //console.log("Warning window!");
                warningText.text = MainClass.currentError.secondItem;
                title = states[MainClass.currentError.firstItem];
                //visible = true;
            }
        }


    }

}
