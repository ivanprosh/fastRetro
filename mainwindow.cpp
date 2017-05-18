#include <QSharedPointer>
#include <QScopedPointer>
#include <QDate>
#include <QSettings>
#include <QTimer>
#include <QFile>
#include <QTextStream>
//#include <QSqlError>

#include "mainwindow.h"
#include "connectionManager.h"
#include "dataanalizator.h"
#include "strategies.h"

namespace {
    //описывают имена свойств для сохранения их в реестре после закрытия программы
    const QString ModelSetting("AddressList");
    const QString AutoStartSetting("AutoStart");
    const QString ServerNameSetting("ServerName");
    const QString RedundantSetting("Redundant");
    const QString RedundancyFilePathSetting("RedundancyFilePath");
    const QString BackupFolderSetting("BackupFolder");
    const QString TimeZoneSetting("TimeZone");
    const QString SegmentIntervalSetting("SegmentInterval");

    const QString _logFileName("CurrentLog.txt");

    //const QString _DBConnectionString("DRIVER={SQL Server};SERVER=%1;DATABASE=RUNTIME;Trusted_Connection=yes");
    //const QString _logFileTemplateString("%1 ");
}

MainWindow::MainWindow(AddressTable *model, QObject *parent):QObject(parent),_model(model),_segmentInterval(1)
{
    //разрешения пользовательских кнопок
    setStartPermit(true);
    setStopPermit(false);
    setSavePermit(true);

    _currentError = new GlobalError(GlobalError::None,QString());

    //logfile.setFileName(_logFileName);
    Logger::instance()->setFile(_logFileName);

    if(_model == nullptr)
        qWarning() << "Некорректная модель данных";
    serverStateNames << "Не подключен" << "Поиск сервера" << "Подключение" << "Подключен" << "Bound to address and port"
                     << "Пользовательский" << "Закрыт";

    //Отдельный поток для отслеживания новых файлов в каталоге BackUp
    currentThread = QSharedPointer<WorkThread >(new WorkThread());
    currentThread->start();

    initObjConnections();
    //initializeSettings();
}
// восстановление значений параметров из реестра
void MainWindow::initializeSettings()
{
    QSettings settings;
    if(!settings.value(ServerNameSetting).toString().isEmpty())
        setServerName(settings.value(ServerNameSetting).toString());
    if(settings.value(SegmentIntervalSetting).toInt()>0)
        setSegmentInterval(settings.value(SegmentIntervalSetting).toInt());
    if(!settings.value(TimeZoneSetting).toString().isEmpty())
        setTimeZone(settings.value(TimeZoneSetting).toString());
    else
        setTimeZone("+3");
    if(!settings.value(BackupFolderSetting).toString().isEmpty())
        setBackupFolderName(settings.value(BackupFolderSetting).toString());
    setRedundant(settings.value(RedundantSetting).toBool());
    if(!settings.value(RedundancyFilePathSetting).toString().isEmpty())
        setRedundancyFilePath(settings.value(RedundancyFilePathSetting).toString());
    if(!settings.value(ModelSetting).toStringList().isEmpty()){
        setAutostart(settings.value(AutoStartSetting).toBool());
        initializeTable(settings.value(ModelSetting).toStringList());
        if(isAutostart())
           QTimer::singleShot(500, this, SLOT(connectToServer()));
    } else {
        setAutostart(0);
    }
}

MainWindow::~MainWindow()
{
    delete _currentError;
}

void MainWindow::setCurrentError(GlobalError *value){

    if(value != nullptr){
        _currentError->setFirstItem(value->firstItem());
        _currentError->setSecondItem(value->secondItem());
        _currentError->setFrom(value->from());
        emit currentErrorChanged(_currentError);
        if(value->firstItem() != GlobalError::None)
            Logger::instance()->addEntry(value);
    } else {
        _currentError->setFirstItem(GlobalError::None);
        _currentError->setSecondItem("Ok");
    }
}

void MainWindow::initObjConnections()
{
    //добавлен клиент
    connect(_model,SIGNAL(SockClientAdd(QSharedPointer<PLCSocketClient>)),
                                this,SLOT(connectClient(QSharedPointer<PLCSocketClient>)));
    //обработка сообщений об ошибках
    connect(DataAnalizator::instance(), SIGNAL(errorChange(GlobalError*)),
            this, SLOT(errorChange(GlobalError*)));
    connect(Logger::instance(), SIGNAL(errorChange(GlobalError*)),
            this, SLOT(errorChange(GlobalError*)));
    connect(currentThread.data(), SIGNAL(errorChange(GlobalError*)),
            this, SLOT(errorChange(GlobalError*)));
    //изменение свойств
    connect(this, SIGNAL(historianPathChanged(const QString&)),
            currentThread.data(), SIGNAL(historianPathChanged(const QString&)));
    connect(this, SIGNAL(backupFolderNameChanged(const QString&)),
            DataAnalizator::instance(), SLOT(setNewFilePath(const QString&)));
    connect(this, SIGNAL(redundancyFilePathChanged(const QString&)),
            DataAnalizator::instance(), SLOT(setRedundancyFilePath(const QString&)));
    connect(this, SIGNAL(redundantChanged(bool)),
            DataAnalizator::instance(), SLOT(setRedundantOption(bool)));
    connect(this, SIGNAL(backupFolderNameChanged(const QString&)),
            currentThread.data(), SIGNAL(backupFolderNameChanged(const QString&)));
    connect(this, SIGNAL(timeZoneChanged(const QString&)),
            DataAnalizator::instance(), SLOT(timeZoneChanged(const QString&)));
    connect(this, SIGNAL(segmentIntervalChanged(int)),
            DataAnalizator::instance(), SLOT(segmentIntervalChanged(int)));
    //
}

void MainWindow::initSockConnections()
{
    foreach (QSharedPointer<PLCSocketClient> client, ConnectionManager::instance()->connections) {
        //именно здесь и добавляется соединение, сделано для универсальности, так как модель данных может быть отредактирована и через пользов. интерфейс
        _model->setData(_model->index(client->server()->id,statusColumn),serverStateNames.first(),statusRole);
    }
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
    //обновление статусной строки
    _model->setData(_model->index(client->server()->id,statusColumn),serverStateNames.at(curState),statusRole);

}

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

void MainWindow::setRedundant(bool value)
{
    _redundant = value;
    QSettings settings;
    settings.setValue(RedundantSetting,value);
    emit redundantChanged(value);
}
//привязка сигналов к слотам для нового сокета
void MainWindow::connectClient(QSharedPointer<PLCSocketClient> client)
{
    connect(client.data(),SIGNAL(stateChanged(QAbstractSocket::SocketState)),
            this,SLOT(socketStateChanged(QAbstractSocket::SocketState)));
    connect(client.data(), SIGNAL(connected()), this, SLOT(connectEstablished()));
    connect(client.data(), SIGNAL(connectionClosedByServer()),
            this, SLOT(connectionClosedByServer()));
    connect(client.data(), SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(error(QAbstractSocket::SocketError)));
    connect(client.data(), SIGNAL(newDataReceived()),
            DataAnalizator::instance(), SLOT(newDataReceived()));
}
void MainWindow::connectEstablished()
{
    updateStateSocket(qobject_cast<PLCSocketClient* >(sender()));
}

void MainWindow::stopClients(){
    // Close all existing connections
    foreach (QSharedPointer<PLCSocketClient> client, ConnectionManager::instance()->connections) {
        stopClients(client->server()->id);
    }
}
void MainWindow::stopClients(int id){
    // Закрываем только одно соединение
    _model->setData(_model->index(id,statusColumn),serverStateNames.first(),statusRole);
    ConnectionManager::instance()->closeConnection(ConnectionManager::instance()->findClient(id));
}
void MainWindow::connectToServer(int plcId)
{
    _model->setData(_model->index(plcId,statusColumn),serverStateNames.first(),statusRole);
    stopClients(plcId);
    ConnectionManager::instance()->activateConnection(ConnectionManager::instance()->findClient(plcId));
}
void MainWindow::connectToServer()
{   
    //initializeServers();
    setStartPermit(false);
    setStopPermit(true);
    setSavePermit(false);

    foreach (QSharedPointer<PLCSocketClient> client, ConnectionManager::instance()->connections) {
        //if(plcIds.contains(QVariant::fromValue(client->getServer()->id)))
        connectToServer(client->getServer()->id);
        //ConnectionManager::instance()->removeConnection(client);
    }

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
//обработчик поступающих ошибок от сокетов
void MainWindow::error(QAbstractSocket::SocketError errCode)
{
    PLCSocketClient* curClient= dynamic_cast<PLCSocketClient* >(sender());
    Q_ASSERT(curClient);
    QScopedPointer<GlobalError> curSocketErr(new GlobalError());

    if(curClient){
        qDebug() << "Socket error type is " << errCode;

        _model->setData(_model->index(curClient->server()->id,statusColumn),"Ошибка:"+curClient->errorString(),statusRole);

        curSocketErr->setFirstItem(GlobalError::Configuration);
        curSocketErr->setFrom(QString(curClient->getServer()->address)+":"
                                +QString::number(curClient->getServer()->port));
        curSocketErr->setPlcIdFrom(curClient->getServer()->id);

        switch (errCode) {
        case QAbstractSocket::HostNotFoundError:
            curSocketErr->setSecondItem(curClient->errorString());
            break;
            //case QAbstractSocket::SocketAccessError:
            //case QAbstractSocket::SocketResourceError:
        case QAbstractSocket::AddressInUseError:
        case QAbstractSocket::SocketAddressNotAvailableError:
        case QAbstractSocket::UnsupportedSocketOperationError:
            //curSocketErr->setFirstItem(GlobalError::Configuration);
            curSocketErr->setSecondItem(curClient->errorString() + ". Проверьте таблицу адресов.");
            //errorChange(curSocketErr.data());
            break;
        case QAbstractSocket::DatagramTooLargeError:
            //curSocketErr->setFirstItem(GlobalError::Configuration);
            curSocketErr->setSecondItem(curClient->errorString() + ". Проверьте размер пакета от ПЛК");
            //errorChange(curSocketErr.data());
            break;
        default:
            curSocketErr->setFirstItem(GlobalError::Socket);
            curSocketErr->setSecondItem(curClient->errorString());
            //errorChange(curSocketErr.data());
            ConnectionManager::instance()->errorHandler(curClient,errCode);

        }
        errorChange(curSocketErr.data());
    }
}
//обработчик всех поступающих ошибок от смежных классов. Здесь принимается решение в зависимости от
//типа ошибки об остановки соединений
void MainWindow::errorChange(GlobalError* lastErr)
{
    switch (lastErr->firstItem()) {
    case GlobalError::Configuration:
        if((lastErr->plcIdFrom() > -1) && (lastErr->plcIdFrom() <= MAX_CONNECTIONS_COUNT)){
            //если заполнено поле idFrom, то тормозим только конкретного клиента
            stopClients(lastErr->plcIdFrom());
        } else
            stopScan();
        //stopClients(int id);
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

void MainWindow::forceReconnect(int rowInTable)
{
    qDebug() << "Force!";
    foreach (QSharedPointer<PLCSocketClient> client, ConnectionManager::instance()->connections) {
        if(client->server()->id == rowInTable){
            setSavePermit(false);
            ConnectionManager::instance()->forceReconnect(client.data());
            break;
        }
    }
}

void MainWindow::forceStartConnect(int rowInTable)
{
    connectToServer(rowInTable);
    setSavePermit(false);
    setStopPermit(true);
}

void MainWindow::forceStopConnect(int rowInTable)
{
    stopClients(rowInTable);
    //если нет активных клиентов, то даем разрешение на редактирование настроек

    foreach (QSharedPointer<PLCSocketClient> client, ConnectionManager::instance()->connections) {
        if(ConnectionManager::instance()->isConnectActive(client.data()))
            return;
    }
    setStopPermit(false);
    setSavePermit(true);

}

void MainWindow::setServerName(QString value){
    //qDebug() << "Server name changed (non confirm)";
    if(_serverName!=value){
        _serverName = value;
        QSettings settings;
        settings.setValue(ServerNameSetting,_serverName);
        emit historianPathChanged(_serverName);
        qDebug() << "Server name confirm changed " << _serverName;
    }
}

void MainWindow::setSegmentInterval(int value)
{
    if(_segmentInterval != value){
        _segmentInterval = value;
        QSettings settings;
        settings.setValue(SegmentIntervalSetting,_segmentInterval);
        emit segmentIntervalChanged(_segmentInterval);
    }
}

void MainWindow::setBackupFolderName(QString value){
    //qDebug() << "Server name changed (non confirm)";
    if(_backupFolderName!=value){
        _backupFolderName = value;
        QSettings settings;
        settings.setValue(BackupFolderSetting,_backupFolderName);
        emit backupFolderNameChanged(_backupFolderName);
        qDebug() << "New backup folder name set - " << _backupFolderName;
    }
}

void MainWindow::setRedundancyFilePath(QString value)
{
    if(_redundancyFilePath!=value){
        _redundancyFilePath = value;
        QSettings settings;
        settings.setValue(RedundancyFilePathSetting,_redundancyFilePath);
        emit redundancyFilePathChanged(_redundancyFilePath);
        qDebug() << "New redundancy file is - " << _redundancyFilePath;
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

auto MainWindow::getLogger() -> Logger* {
    return Logger::instance();
}
