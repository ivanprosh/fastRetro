#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "addresstable.h"
#include "plcsocketclient.h"
#include "global.h"
#include "globalerror.h"
#include "workthread.h"
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
    Q_PROPERTY(QString serverName READ serverName WRITE setServerName NOTIFY serverNameChanged)
    Q_PROPERTY(QString backupFolderName READ backupFolderName WRITE setBackupFolderName NOTIFY backupFolderNameChanged)
    Q_PROPERTY(GlobalError* currentError READ currentError WRITE setCurrentError NOTIFY currentErrorChanged)

public:
    MainWindow(AddressTable* model,QObject *parent = 0);
    ~MainWindow();

    void setStartPermit(bool value){permitStart = value; emit startPermitChanged();}
    void setStopPermit(bool value){permitStop = value; emit stopPermitChanged();}
    void setSavePermit(bool value){permitSave = value; emit savePermitChanged();}
    void setCurrentError(GlobalError* value);
    void initializeSettings();

public slots:
    void socketStateChanged(QAbstractSocket::SocketState curState);
    void saveConfig();
    void setAutostart(bool value);
    bool isAutostart() {return _autostart;}
    //
    void connectToServer();
    void stopScan();
    void connectionClosedByServer();
    void connectEstablished();
    void error();
    void errorChange(GlobalError* lastErr );
    void resetError();

    bool startPermit() {return permitStart;}
    bool stopPermit() {return permitStop;}
    bool savePermit() {return permitSave;}

    void setServerName(QString value);
    QString serverName() {return _serverName;}

    void setBackupFolderName(QString value);
    QString backupFolderName() {return _backupFolderName;}

    GlobalError* currentError(){return _currentError;} 
    void setDB(const QString &server);

private:
    void initObjConnections();
    void initSockConnections();

    void initializeServers();
    void initializeTable(const QStringList& list);
    void stopClients();
    void connectClient(QSharedPointer<PLCSocketClient> client);
    //Обновляет запись в таблице подключений
    void updateStateSocket(PLCSocketClient *client);
    QString getlastErrorDB();

    AddressTable* _model;
    QList<QSharedPointer<PLCServer> > servers;
    QStringList serverStateNames;
    bool permitStart,permitStop,permitSave,_autostart;
    QString _serverName, _backupFolderName;
    GlobalError* _currentError;
    QSharedPointer<WorkThread> currentThread;
    QFile logfile;
    QTextStream logFileStream;
    //QList<PLCSocketClient> clients;

signals:
    void startPermitChanged();
    void stopPermitChanged();
    void savePermitChanged();
    void autostartChanged();
    void serverNameChanged(const QString&);
    void backupFolderNameChanged(const QString&);
    void currentErrorChanged(GlobalError*);
};

#endif // MAINWINDOW_H
