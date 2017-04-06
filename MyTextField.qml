import QtQuick 2.2
import QtQuick.Window 2.2
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.2
import QtQuick.Particles 2.0

TextField {
    style: TextFieldStyle {
        background: BorderImage {
            source: "images/textfield.png"
            border.left: 4 ; border.right: 4 ; border.top: 4 ; border.bottom: 4
        }
    }
    //implicitWidth: window.width / columnFactor
//    Component {
//        id: textFieldStyle
//        TextFieldStyle {
//            background: Rectangle {
//                implicitWidth: window.width / columnFactor
//                color: "#f0f0f0"
//                antialiasing: true
//                border.color: "gray"
//                radius: height/2
//                Rectangle {
//                    anchors.fill: parent
//                    anchors.margins: 1
//                    color: "transparent"
//                    antialiasing: true
//                    border.color: "#aaffffff"
//                    radius: height/2
//                }
//            }
//        }
//    }
}



