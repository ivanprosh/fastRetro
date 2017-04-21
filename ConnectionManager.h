#ifndef TRIPPLANNER_H
#define TRIPPLANNER_H

//#include <QDialog>
#include <QSharedPointer>
#include "plcsocketclient.h"
#include "global.h"

class ConnectionManager : public QObject//
{
    Q_OBJECT

public:
    static ConnectionManager *instance();

    bool canAddConnection() const;
    void addConnection(QSharedPointer<PLCSocketClient> connection);
    void removeConnection(QSharedPointer<PLCSocketClient> connection);
    int maxConnections() const;
    QByteArray clientId() const;
    void errorHandler(PLCSocketClient *curClient, QAbstractSocket::SocketError);
    void closeConnection(PLCSocketClient *curClient);
    void activateConnection(PLCSocketClient *curClient);
    void forceReconnect(PLCSocketClient *curClient);
    //void forceStopConnect(PLCSocketClient *curClient);

    //ConnectionManager(QAbstractTableModel* model,QObject *parent = 0);
protected:
    QSet<QSharedPointer<PLCSocketClient> > connections;
private:
    friend class MainWindow;

    //QTcpSocket tcpSocket;
    mutable QByteArray id;
};

#endif
