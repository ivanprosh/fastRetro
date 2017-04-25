import QtQuick 2.1
import QtQuick.Controls 2.1
import QtQuick.Dialogs 1.2 as AppDialogs
import QtQuick.Layouts 1.1

//Component {
//    id: filedialog

AppDialogs.FileDialog {
    property var context
    //id: dialog
    //options: FolderDialog.DontResolveSymlinks | FolderDialog.ReadOnly
    //selectFolder: true
    title: "Выберите путь до каталога хранения временных файлов"
    //currentFolder: shortcuts.home

    onAccepted: {
        console.log("You chose: " + folder)
        var url = folder.toString();
        if (!selectFolder){
            url = fileUrl.toString();
            console.log(url);
        }
        var path = url.match('file\:///(.*)');
        if(path !== null)
            context.text = path[1];
        else
            context.placeholderText = "Некорректный путь...";

        loader.active = false;
    }
    onRejected: {
        console.log("Canceled")
        loader.active = false
    }
    Component.onCompleted: visible = true
}
//}
