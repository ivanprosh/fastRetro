#include <QtEndian>
#include <QDataStream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mstcpip.h>
#include <windows.h>

#include "plcsocketclient.h"
#include "global.h"

#pragma	 comment (lib,"ws2_32.lib")

//static struct tcp_keepalive vals;

namespace {
    static const int ConnectTimeout = 60 * 1000;
    static const int SizeRecieveBuf = 1000;
    static const int SizeCyclicQue = 300;
    static const int MaxPacketSize = 1000;
    //const int reconnectDelay = 5000;
}

int Packet::count = 0;

PLCSocketClient::PLCSocketClient(const QByteArray &Id, QObject *parent):
    QTcpSocket(parent),
    invalidateTimeout(false),
    nextBlockSize(0), reconnectDelay(1000),reconnectCount(0),
    queReceivePackets(SizeCyclicQue)
{
    //timeoutTimer = startTimer(ConnectTimeout);
    sockIdString = Id;

/*
    DWORD dwBytesRet = 0;
    DWORD dwRet;
*/
    setSocketOption(QAbstractSocket::KeepAliveOption, 1);
/*
    int descriptor = socketDescriptor();

    dwRet = WSAIoctl(descriptor, SIO_KEEPALIVE_VALS, &vals, sizeof(vals), NULL, 0, &dwBytesRet, NULL, NULL);
*/
    reconnectTimer = QSharedPointer<QTimer>(new QTimer());
    reconnectTimer->setInterval(reconnectDelay);
    reconnectTimer->setSingleShot(true);
/*
    keepAliveTimer = QSharedPointer<QTimer>(new QTimer());
    keepAliveTimer->setInterval(reconnectDelay);
    keepAliveTimer->setSingleShot(true);
*/
    connect(this, SIGNAL(readyRead()),SLOT(newDataAvailable()));
    connect(this, SIGNAL(connected()), SLOT(connectEstablished()));
    connect(this, SIGNAL(disconnected()), SLOT(closeConnection()));
    connect(reconnectTimer.data(), SIGNAL(timeout()), this, SLOT(reconnect()));

    //connect(this, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(error()));

    //connect(this, SIGNAL(readyRead()), this, SIGNAL(readyToTransfer()));
    //connect(this, SIGNAL(connected()), this, SIGNAL(readyToTransfer()));

//    connect(&socket, SIGNAL(connected()),
//            this, SIGNAL(connected()));
//    connect(&socket, SIGNAL(readyRead()),
//            this, SIGNAL(readyRead()));
//    connect(&socket, SIGNAL(disconnected()),
//            this, SIGNAL(disconnected()));
//    connect(&socket, SIGNAL(error(QAbstractSocket::SocketError)),
//            this, SIGNAL(error(QAbstractSocket::SocketError)));
//    connect(&socket, SIGNAL(bytesWritten(qint64)),
//            this, SIGNAL(bytesWritten(qint64)));
//    connect(&socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
//            this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
}

PLCSocketClient::~PLCSocketClient(){
    //delete[] recieveBuf;
    this->close();
    this->disconnect();
    qDebug() << "~SockClient";
}
//void PLCSocketClient::timerEvent(QTimerEvent *event)
//{
//    if (event->timerId() == timeoutTimer) {
//        // Disconnect if we timed out; otherwise the timeout is
//        // restarted.
//        if (invalidateTimeout) {
//            invalidateTimeout = false;
//        } else {
//            qDebug() << "timeoutTimer finish!"
//            //abort();
//        }
//    }
//    QTcpSocket::timerEvent(event);
//}
void PLCSocketClient::setServer(const QSharedPointer<PLCServer>& plc)
{
    _plcServer = plc;
}

QSharedPointer<PLCServer> PLCSocketClient::getServer()
{
    return _plcServer;
}

QSharedPointer<PLCServer> PLCSocketClient::server() const
{
    return _plcServer;
}

void PLCSocketClient::newDataAvailable()
{
    QDataStream in(this);
    invalidateTimeout = true;

    if (nextBlockSize == 0) {
        if (this->bytesAvailable() < sizeof(quint32))
            return;
        quint32 size;
        in >> size;
        nextBlockSize = qFromBigEndian(size);
        //qDebug() << "Size is " << nextBlockSize;
    }

    if (this->bytesAvailable() < (nextBlockSize - sizeof(quint32))){
        qDebug() << "low bytes available: " << this->bytesAvailable();
        return;
    }
    //if(nextBlockSize != 440)
        //qDebug() << this->bytesAvailable();


    if(nextBlockSize < MaxPacketSize && nextBlockSize > 0) {

        //создаем структуру для хранения одного пакета данных
        QSharedPointer<Packet> curPacket(new Packet(nextBlockSize - sizeof(quint32)));

        in.startTransaction();

        this->_plcServer->lastVisited = QDateTime::currentDateTime().toTime_t();

        //qDebug() << "before read " << this->bytesAvailable();
        //считывание потока в контейнер
        for(int it=0;it<(nextBlockSize-sizeof(quint32))*0.25;it++){
            in >> curPacket->data[it].u;
            curPacket->data[it].u = qFromBigEndian(curPacket->data[it].u);
            //qDebug() << curPacket->data[it].f;
        }
        //qDebug() << "after read " << this->bytesAvailable();
        if(!in.commitTransaction()) {
            //qDebug() << "Transaction fault! Status:" << in.status();
            return;
        }
        //curPacket->setTime((int)curPacket->getValue(0),(int)curPacket->getValue(1),(int)curPacket->getValue(2),(int)(curPacket->getValue(3)/1000));
        curPacket->setTime();
        curPacket->setDate();

        //nextBlockSizeUint = 0;
        nextBlockSize = 0;
        //добавляем пакет в очередь
        //if(!queReceivePackets.isEmpty()) queReceivePackets.pop_front();

        queReceivePackets.enqueue(curPacket);

        //qDebug() << this->bytesAvailable();
        emit newDataReceived();
    } else {
        qDebug() << "PLCSocketClient:: Wrong Packet Size!";
        readAll();
    }
}

void PLCSocketClient::connectEstablished()
{
    //активируем keep-alive
    tcp_keepalive vals;
    vals.onoff             = 1;    // non-zero means "enable"
    vals.keepalivetime     = 5000;   // milliseconds
    vals.keepaliveinterval = 1000;  // milliseconds

    DWORD dwBytesRet = 0;
    DWORD dwErr = 0;
    if( WSAIoctl(this->socketDescriptor(), SIO_KEEPALIVE_VALS, &vals, sizeof(vals), NULL, 0, &dwBytesRet, NULL, NULL) !=0 ) {
        dwErr = WSAGetLastError();
        qWarning() << (char*)dwErr;
    }

    readAll();
    stopReconnectTimer();
    qDebug() << "PLCSocketClient:: id " << this->_plcServer->id << " Соединение установлено";
}
void PLCSocketClient::closeConnection()
{
    this->abort();
}
void PLCSocketClient::startReconnectTimer()
{
    //QTimer::singleShot(reconnectDelay,SLOT(reconnect()));
    reconnectTimer->setInterval(reconnectDelay);
    reconnectTimer->start();
}

void PLCSocketClient::stopReconnectTimer()
{
    reconnectCount = 0;
    reconnectDelay = 1000;
    reconnectTimer->stop();
}

bool PLCSocketClient::isActive()
{
    //if(reconnectTimer.data())
    return (reconnectTimer.data()->isActive() || this->isOpen());
    //return this->isOpen();
    //return true;
}
void PLCSocketClient::reconnect()
{
    qDebug() << "PLCSocketClient:: Try to reconnect number " << reconnectCount;
    this->connectToHost(_plcServer->address, _plcServer->port);
    reconnectDelay = GLOBAL::Fib(++reconnectCount)*1000;
}
