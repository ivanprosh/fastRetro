import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Particles 2.0

ApplicationWindow {
    id: window
    width: 640
    height: 480
    visible: true
    title: qsTr("FastRetro App")

    toolBar: ToolBar {
        RowLayout{
            Button {
                id: startButton
                enabled: MainClass.startPermit
                text: "Start"
                implicitWidth: window.width / columnFactor
                anchors.left: parent.left
                onClicked: MainClass.connectToServer()
            }
            Button {
                id: stopButton
                enabled: MainClass.stopPermit
                text: "Stop"
                implicitWidth: window.width / columnFactor
                onClicked: MainClass.stopScan()
            }
        }
        MyTextField {
            id: applicationStatus
            readOnly: true

            placeholderText: "Текущее состояние..."
            Layout.fillWidth: true
            //inputMethodHints: Qt.ImhNoPredictiveText

            width: window.width / 5 * 2
            anchors.right: parent.right
            //anchors.verticalCenter: parent.verticalCenter

        }

    }
    property int columnFactor: 5

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
                    if(data == "Подключение")
                        return "green"
                    if(data.indexOf('Ошибка',0) !== -1)
                        return "red"
                    else
                        return color
                }

                Text {
                    width: parent.width
                    anchors.margins: 4
                    anchors.left: parent.left
                    anchors.verticalCenter: parent.verticalCenter
                    elide: styleData.elideMode
                    text: styleData.value !== undefined ? styleData.value : ""
                    color : styleData.textColor
                    visible: !styleData.selected
                    onTextChanged: {
                       console.log("Changing text")
                       this.color = setColor(styleData.value,styleData.textColor)
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
                        }
                    }
                    sourceComponent: styleData.selected ? editor : null
                    Component {
                        id: editor
                        TextInput {
                            id: textinput
                            color: styleData.textColor
                            text: styleData.value
                            MouseArea {
                                id: mouseArea
                                enabled: styleData.column == 0
                                anchors.fill: parent
                                hoverEnabled: true
                                onClicked: textinput.forceActiveFocus()
                            }
                        }
                    }
                }
            }
        }
    }

    /*
    GridLayout {
        rowSpacing: 12
        columnSpacing: 30
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.margins: 30

        MyTextField {
            Layout.row: 1
            implicitWidth: window.width / columnFactor
        }
        MyTextField {
            readOnly: true
            implicitWidth: window.width / columnFactor
        }
        CheckBox {
            text: "Активен"
            implicitWidth: window.width / columnFactor
        }

        ProgressBar {
            Layout.row: 2
            value: slider1.value
            implicitWidth: window.width / columnFactor
        }


    }
    */
}
