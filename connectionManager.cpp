#include <QtGui>
#include <QtNetwork>

#include "connectionManager.h"

//static const int MaxConnections = 10;

Q_GLOBAL_STATIC(ConnectionManager, connectionManager)

ConnectionManager *ConnectionManager::instance()
{
    return connectionManager();
}

bool ConnectionManager::canAddConnection() const
{
    return (connections.size() < MAX_CONNECTIONS_COUNT);
}

void ConnectionManager::addConnection(QSharedPointer<PLCSocketClient> client)
{
    connections << client;
    //initializeConnection(client);
}

void ConnectionManager::removeConnection(QSharedPointer<PLCSocketClient> client)
{
    connections.remove(client);
}

int ConnectionManager::maxConnections() const
{
    return MAX_CONNECTIONS_COUNT;
}

QByteArray ConnectionManager::clientId() const
{
    if (id.isEmpty()) {
        // Generate client id
        int startupTime = int(QDateTime::currentDateTime().toTime_t());

        //id += QString("-QT%1-").arg((QT_VERSION % 0xffff00) >> 8).toLatin1();
        id += QByteArray::number(startupTime, 10);
        //id += QByteArray(20 - id.size(), '-');
    }
    //qDebug() << "Current Client Sock id is " << id;
    return id;
}

