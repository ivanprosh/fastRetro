#include <QSharedPointer>
#include <QScopedPointer>
#include <QDate>
#include <QSettings>
#include <QTimer>
#include <QFile>
#include <QTextStream>
#include <QSqlError>

#include "mainwindow.h"
#include "connectionManager.h"
#include "dataanalizator.h"

namespace {
    const QString ModelSetting("AddressList");
    const QString AutoStartSetting("AutoStart");
    const QString ServerNameSetting("ServerName");
    const QString BackupFolderSetting("BackupFolder");
    const QString TimeZoneSetting("TimeZone");

    const QString _logFileName("CurrentLog.txt");

    //const QString _DBConnectionString("DRIVER={SQL Server};SERVER=%1;DATABASE=RUNTIME;Trusted_Connection=yes");
    //const QString _logFileTemplateString("%1 ");
}

MainWindow::MainWindow(AddressTable *model, QObject *parent):QObject(parent),_model(model)
{
    setStartPermit(true);
    setStopPermit(false);
    setSavePermit(true);

    _currentError = new GlobalError(GlobalError::None,QString());

    //logfile.setFileName(_logFileName);
    Logger::instance()->setFile(_logFileName);
/*
    if(!logfile.open(QIODevice::WriteOnly | QIODevice::Text)){
        QScopedPointer<GlobalError> lastErr(new GlobalError(GlobalError::System,
                                            "Ошибка открытия лог файла"));
        errorChange(lastErr.data());
    } else {
        logFileStream.setDevice(&logfile);
    }
*/

    if(_model == nullptr)
        qWarning() << "Некорректная модель данных";
    serverStateNames << "Не подключен" << "Поиск сервера" << "Подключение" << "Подключен" << "Bound to address and port"
                     << "Пользовательский" << "Закрыт";

    //Отдельный поток для отслеживания новых файлов в каталоге BackUp
    currentThread = QSharedPointer<WorkThread>(new WorkThread());
    currentThread->start();

    initObjConnections();
    //initializeSettings();
}

void MainWindow::initializeSettings()
{
    QSettings settings;
    if(!settings.value(ServerNameSetting).toString().isEmpty())
        setServerName(settings.value(ServerNameSetting).toString());
    if(!settings.value(TimeZoneSetting).toString().isEmpty())
        setTimeZone(settings.value(TimeZoneSetting).toString());
    else
        setTimeZone("+3");
    if(!settings.value(BackupFolderSetting).toString().isEmpty())
        setBackupFolderName(settings.value(BackupFolderSetting).toString());
    if(!settings.value(ModelSetting).toStringList().isEmpty()){
        setAutostart(settings.value(AutoStartSetting).toBool());
        initializeTable(settings.value(ModelSetting).toStringList());
        if(isAutostart())
           QTimer::singleShot(0, this, SLOT(connectToServer()));
    } else {
        setAutostart(0);
    }
}

MainWindow::~MainWindow()
{
    delete _currentError;
}

void MainWindow::setCurrentError(GlobalError *value){
    if(_currentError->secondItem()!=value->secondItem()){
        _currentError->setFirstItem(value->firstItem());
        _currentError->setSecondItem(value->secondItem());
        emit currentErrorChanged(_currentError);
        if(value->firstItem() != GlobalError::None)
            Logger::instance()->addEntry(value);
    }
}

void MainWindow::initObjConnections()
{
    connect(this, SIGNAL(serverNameChanged(const QString&)),
            currentThread.data(), SIGNAL(serverNameChanged(const QString&)));
    connect(DataAnalizator::instance(), SIGNAL(errorChange(GlobalError*)),
            this, SLOT(errorChange(GlobalError*)));
    connect(Logger::instance(), SIGNAL(errorChange(GlobalError*)),
            this, SLOT(errorChange(GlobalError*)));
    connect(this, SIGNAL(backupFolderNameChanged(const QString&)),
            DataAnalizator::instance(), SLOT(setNewFilePath(const QString&)));
    connect(this, SIGNAL(backupFolderNameChanged(const QString&)),
            currentThread.data(), SIGNAL(backupFolderNameChanged(const QString&)));
    connect(currentThread.data(), SIGNAL(errorChange(GlobalError*)),
            this, SLOT(errorChange(GlobalError*)));
    connect(this, SIGNAL(timeZoneChanged(const QString&)),
            DataAnalizator::instance(), SLOT(timeZoneChanged(const QString&)));
}

void MainWindow::initSockConnections()
{
    foreach (QSharedPointer<PLCSocketClient> client, ConnectionManager::instance()->connections) {
        _model->setData(_model->index(client->server()->id,statusColumn),"Не активен",statusRole);
        //client->close();
        ConnectionManager::instance()->removeConnection(client);
    }
   ConnectionManager::instance()->connections.clear();
}

void MainWindow::initializeTable(const QStringList& list){
    int row(0);
    while(row < list.size()) {
        _model->ipChange(row,ipColumn,list.at(row));
        row++;
    }
}

void MainWindow::updateStateSocket(PLCSocketClient* client){
    if(!client){
        qDebug() << "Pointer is null! updateStateSocket()";
        return;
    }
    QAbstractSocket::SocketState curState = client->state();
    Q_ASSERT(curState < QAbstractSocket::ClosingState+1);

    //if(client->errorString().isEmpty())
    _model->setData(_model->index(client->server()->id,statusColumn),serverStateNames.at(curState),statusRole);
    //else
    //    _model->setData(_model->index(client->server()->id,statusColumn),serverStateNames.at(curState)+",ошибка:"+client->errorString(),statusRole);

}
/*
void MainWindow::setDB(const QString &server)
{
    qDebug() << "MainWindow::Server change name: " << server;

    QMutexLocker locker(&GLOBAL::globalMutex);

    if(currentThread==Q_NULLPTR) {
        qDebug() << "currentThread is NULL!";
        return;
    }

    //QString stringConnection="DRIVER={SQL Server};SERVER=%1;DATABASE=RUNTIME;UID=fastRetroUser;PWD=1;Trusted_Connection=no; WSID=.";
    currentThread->setConnection(QString(_DBConnectionString).arg(server));


    if(currentThread->getWorker()->getDB().open()){
        qDebug() << "DataAnalizator::DB open!";
        if(!currentThread->isRunning())
            ;
        return;
    }  

}
*/
void MainWindow::socketStateChanged(QAbstractSocket::SocketState curState)
{
    updateStateSocket(qobject_cast<PLCSocketClient* >(sender()));
}

void MainWindow::saveConfig()
{
    QSettings settings;
    int curRow = 0;
    QString note;
    QStringList Addresses;
    while(!(note=_model->index(curRow,ipColumn).data(ipRole).toString()).isEmpty()){
        Addresses << note;
        curRow++;
    }
    settings.setValue(ModelSetting,Addresses);
    settings.setValue(ServerNameSetting,serverName());
}

void MainWindow::setAutostart(bool value){
    _autostart = value;
    QSettings settings;
    settings.setValue(AutoStartSetting,value);
    emit autostartChanged();
}

void MainWindow::connectClient(QSharedPointer<PLCSocketClient> client)
{
    connect(client.data(),SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this,SLOT(socketStateChanged(QAbstractSocket::SocketState)));
    connect(client.data(), SIGNAL(connected()), this, SLOT(connectEstablished()));
    connect(client.data(), SIGNAL(connectionClosedByServer()),
            this, SLOT(connectionClosedByServer()));
    connect(client.data(), SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(error()));
    connect(client.data(), SIGNAL(newDataReceived()),
            DataAnalizator::instance(), SLOT(newDataReceived()));
}
void MainWindow::connectEstablished()
{
    updateStateSocket(qobject_cast<PLCSocketClient* >(sender()));
}

void MainWindow::initializeServers()
{
    servers.clear();
    int curRow(0);
    QString note;
    //QString note = _model->data(_model->index(curRow,ipColumn),ipRole).toString();
    qDebug() << "инициализация списка серверов";

    while(!(note=_model->index(curRow,ipColumn).data(ipRole).toString()).isEmpty()){
         qDebug() << note.left(note.indexOf(":")) << " and port " << note.right(note.size()-note.indexOf(":")-1);

         //инициализация серверов
         QSharedPointer<PLCServer> plc = QSharedPointer<PLCServer>(new PLCServer);

         plc->id = curRow;
         plc->address = note.left(note.indexOf(":"));
         plc->port =  note.right(note.size()-note.indexOf(":")-1).toUInt();

         servers.push_back(plc);

         curRow++;
    }
}
void MainWindow::stopClients(){
    // Close all existing connections
    foreach (QSharedPointer<PLCSocketClient> client, ConnectionManager::instance()->connections) {
        _model->setData(_model->index(client->server()->id,statusColumn),"Не активен",statusRole);
        client->close();
        //ConnectionManager::instance()->removeConnection(client);
    }
    //ConnectionManager::instance()->connections.clear();
}

void MainWindow::connectToServer()
{
    setStartPermit(false);
    setStopPermit(true);
    setSavePermit(false);

    initializeServers();
    stopClients();
    initSockConnections();
    //clients.clear();

    foreach (QSharedPointer<PLCServer> plc, servers) {
        if(!ConnectionManager::instance()->canAddConnection()) {
            qDebug() << "Достигнут максимум подключений";
            break;
        }
        QSharedPointer<PLCSocketClient> client = QSharedPointer<PLCSocketClient>(new PLCSocketClient(ConnectionManager::instance()->clientId()));
        ConnectionManager::instance()->addConnection(client);
        client->setServer(plc);
        connectClient(client);

        plc->cycleStep = 20;
        plc->connectStart = QDateTime::currentDateTime().toTime_t();
        plc->lastVisited = plc->connectStart;
        DataAnalizator::instance()->ignorePLClist[plc->id] = false;
        client->connectToHost(plc->address, plc->port);
    }

    //nextBlockSize = 0;
}
void MainWindow::stopScan()
{
    stopClients();
    setStartPermit(true);
    setStopPermit(false);
    setSavePermit(true);
}

void MainWindow::connectionClosedByServer()
{
    qDebug() << "MainClass Connect close by server...";
}

void MainWindow::error()
{
    PLCSocketClient* curClient= dynamic_cast<PLCSocketClient* >(sender());
    Q_ASSERT(curClient);
    QString curStatus = _model->data(_model->index(curClient->server()->id,statusColumn),statusRole).toString();
    _model->setData(_model->index(curClient->server()->id,statusColumn),curStatus+",ошибка:"+curClient->errorString(),statusRole);
    //closeConnection();
}

void MainWindow::errorChange(GlobalError* lastErr)
{
    //qDebug() << "New error: " << lastErr;
    //QScopedPointer<GlobalError> CurError(new GlobalError(GlobalError::Historian,lastErr));
    switch (lastErr->firstItem()) {
    case GlobalError::Configuration:
        stopScan();
        //GLOBAL::warning(0,0,"Ошибка конфигурации",lastErr->secondItem());
        break;
    case GlobalError::Historian:
        break;
    case GlobalError::System:
        break;
    default:
        break;
    }
    setCurrentError(lastErr);
}

void MainWindow::resetError()
{
    QScopedPointer<GlobalError> CurError(new GlobalError(GlobalError::None,"Ок"));
    setCurrentError(CurError.data());
}

void MainWindow::setServerName(QString value){
    //qDebug() << "Server name changed (non confirm)";
    if(_serverName!=value){
        _serverName = value;
        QSettings settings;
        settings.setValue(ServerNameSetting,_serverName);
        emit serverNameChanged(_serverName);
        qDebug() << "Server name confirm changed " << _serverName;
    }
}

void MainWindow::setBackupFolderName(QString value){
    //qDebug() << "Server name changed (non confirm)";
    if(_backupFolderName!=value){
        _backupFolderName = value;
        QSettings settings;
        settings.setValue(BackupFolderSetting,_backupFolderName);
        emit backupFolderNameChanged(_backupFolderName);
    }
}

void MainWindow::setTimeZone(QString value)
{
    if(QString::number(_timeZone)!=value){
        _timeZone = value.toInt();
        QSettings settings;
        settings.setValue(TimeZoneSetting,_timeZone);
        emit timeZoneChanged(QString::number(_timeZone));
    }
}

QString MainWindow::getlastErrorDB()
{
    return currentThread->getWorker()->getDB().lastError().text();
    //return QString();
}
auto MainWindow::getLogger() -> Logger* {
    return Logger::instance();
}
