#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "addresstable.h"
#include "plcsocketclient.h"
#include "global.h"
#include "globalerror.h"
#include "workthread.h"
#include "logger.h"
#include <QObject>

//Q_DECLARE_METATYPE(GlobalError)
class QFile;

class MainWindow: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool startPermit READ startPermit WRITE setStartPermit NOTIFY startPermitChanged)
    Q_PROPERTY(bool stopPermit READ stopPermit WRITE setStopPermit NOTIFY stopPermitChanged)
    Q_PROPERTY(bool savePermit READ savePermit WRITE setSavePermit NOTIFY savePermitChanged)
    Q_PROPERTY(bool autostart READ isAutostart WRITE setAutostart NOTIFY autostartChanged)
    Q_PROPERTY(int segmentInterval READ segmentInterval WRITE setSegmentInterval NOTIFY segmentIntervalChanged)
    Q_PROPERTY(QString serverName READ serverName WRITE setServerName NOTIFY historianPathChanged)
    Q_PROPERTY(QString timeZone READ timeZone WRITE setTimeZone NOTIFY timeZoneChanged)
    Q_PROPERTY(QString backupFolderName READ backupFolderName WRITE setBackupFolderName NOTIFY backupFolderNameChanged)
    Q_PROPERTY(Logger* logger READ getLogger CONSTANT)
    Q_PROPERTY(GlobalError* currentError READ currentError WRITE setCurrentError NOTIFY currentErrorChanged)

public:
    MainWindow(AddressTable* model,QObject *parent = 0);
    ~MainWindow();

    void setStartPermit(bool value){permitStart = value; emit startPermitChanged();}
    void setStopPermit(bool value){permitStop = value; emit stopPermitChanged();}
    void setSavePermit(bool value){permitSave = value; emit savePermitChanged();}
    void setCurrentError(GlobalError* value);
    auto getLogger() -> Logger*;

public slots:
    void socketStateChanged(QAbstractSocket::SocketState curState);
    void saveConfig();
    void setAutostart(bool value);
    bool isAutostart() {return _autostart;}
    void initializeSettings();
    //
    void connectToServer();
    void stopScan();
    void connectionClosedByServer();
    void connectEstablished();
    void error(QAbstractSocket::SocketError);
    void errorChange(GlobalError* lastErr );
    void resetError();
    void forceReconnect(int rowInTable);
    void forceStartConnect(int rowInTable);
    void forceStopConnect(int rowInTable);

    bool startPermit() {return permitStart;}
    bool stopPermit() {return permitStop;}
    bool savePermit() {return permitSave;}

    void setServerName(QString value);
    void setSegmentInterval(int value);
    void setBackupFolderName(QString value);
    void setTimeZone(QString value);

    QString timeZone() {return QString::number(_timeZone);}
    QString backupFolderName() { return _backupFolderName; }
    QString serverName() {return _serverName;}
    int segmentInterval() {return _segmentInterval;}

    GlobalError* currentError(){return _currentError;} 

private:
    void initObjConnections();
    void initSockConnections();

    void initializeServers();
    void initializeTable(const QStringList& list);
    void stopClients();
    void connectClient(QSharedPointer<PLCSocketClient> client);
    //Обновляет запись в таблице подключений
    void updateStateSocket(PLCSocketClient *client);

    AddressTable* _model;
    QList<QSharedPointer<PLCServer> > servers;
    QStringList serverStateNames;
    bool permitStart,permitStop,permitSave,_autostart;
    QString _serverName, _backupFolderName;
    GlobalError* _currentError;

    QSharedPointer<WorkThread > currentThread;

    int _timeZone,_segmentInterval;

    //QList<PLCSocketClient> clients;

    void updateServer(int idServer);
signals:
    void segmentIntervalChanged(int);
    void startPermitChanged();
    void stopPermitChanged();
    void savePermitChanged();
    void autostartChanged();
    void historianPathChanged(const QString&);
    void backupFolderNameChanged(const QString&);
    void timeZoneChanged(const QString&);
    void currentErrorChanged(GlobalError*);
};

#endif // MAINWINDOW_H
