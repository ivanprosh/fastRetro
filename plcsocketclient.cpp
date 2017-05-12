#include <QtEndian>
#include <QDataStream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <mstcpip.h>
#include <windows.h>

#include "plcsocketclient.h"
#include "global.h"
#include "logger.h"

#pragma	 comment (lib,"ws2_32.lib")

//static struct tcp_keepalive vals;

namespace {
    static const int ConnectTimeout = 60 * 1000;
    static const int SizeRecieveBuf = 1000;
    static const int SizeCyclicQue = 300;
    static const int MaxPacketSize = 1000;
    //const int reconnectDelay = 5000;
}

//int Packet::count = 0;

PLCSocketClient::PLCSocketClient(const QByteArray &Id, QObject *parent):
    QTcpSocket(parent),
    invalidateTimeout(false),
    nextBlockSize(0), reconnectDelay(1000),reconnectCount(0),
    queReceivePackets(SizeCyclicQue)
{
    //timeoutTimer = startTimer(ConnectTimeout);
    sockIdString = Id;

    setSocketOption(QAbstractSocket::KeepAliveOption, 1);
/*
    int descriptor = socketDescriptor();

    dwRet = WSAIoctl(descriptor, SIO_KEEPALIVE_VALS, &vals, sizeof(vals), NULL, 0, &dwBytesRet, NULL, NULL);
*/
    reconnectTimer = QSharedPointer<QTimer>(new QTimer());
    reconnectTimer->setInterval(reconnectDelay);
    reconnectTimer->setSingleShot(true);

    keepAliveTimer = QSharedPointer<QTimer>(new QTimer());
    keepAliveTimer->setInterval(5000);
    keepAliveTimer->setSingleShot(true);

    connect(this, SIGNAL(readyRead()),SLOT(newDataAvailable()));
    connect(this, SIGNAL(connected()), SLOT(connectEstablished()));
    connect(this, SIGNAL(disconnected()), SLOT(closeConnection()));
    connect(reconnectTimer.data(), SIGNAL(timeout()), this, SLOT(reconnect()));

#ifdef DEBUGLOG
    connect(keepAliveTimer.data(), SIGNAL(timeout()), this, SLOT(isAlive()));
#endif
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
        //nextBlockSize = size;
        //if(nextBlockSize != 440 && nextBlockSize != 72)
            qDebug() << "Size is " << nextBlockSize;
    }

    if (this->bytesAvailable() < (nextBlockSize - sizeof(quint32))){
        qDebug() << "low bytes available: " << this->bytesAvailable();
        return;
    }

    if(nextBlockSize < MaxPacketSize && nextBlockSize > 0) {

        //создаем структуру для хранения одного пакета данных
        QSharedPointer<Packet> curPacket(new Packet(nextBlockSize));

        in.startTransaction();

        this->_plcServer->lastVisited = QDateTime::currentDateTime().toTime_t();
        //резервный инт для выравнивания до четного числа INT'ов.
        quint16 reserv;
        //считываем заголовочные параметры
        in >> curPacket->Year >> curPacket->Month >> curPacket->Day >>
              curPacket->Hour >> curPacket->Minute >> curPacket->Second >> curPacket->MSecond >>
              curPacket->ParCount >> curPacket->CyclesCount >> reserv;
        //конвертация из BigEndian (применяется, так как AB передает данные в этом формате, а переворачивать проще здесь)
        curPacket->Year = qFromBigEndian(curPacket->Year);
        curPacket->Month = qFromBigEndian(curPacket->Month);
        curPacket->Day = qFromBigEndian(curPacket->Day);
        curPacket->Hour = qFromBigEndian(curPacket->Hour);
        curPacket->Minute = qFromBigEndian(curPacket->Minute);
        curPacket->Second = qFromBigEndian(curPacket->Second);
        curPacket->MSecond = qFromBigEndian(curPacket->MSecond);
        curPacket->ParCount = qFromBigEndian(curPacket->ParCount);
        curPacket->CyclesCount = qFromBigEndian(curPacket->CyclesCount);

        qDebug() << "Packet:: parCount, Cycles " << curPacket->ParCount << " " << curPacket->CyclesCount;
        //считывание потока в контейнер
        for(int it=0;it<curPacket->getDataSize();it++){
            in >> curPacket->data[it].u;
            curPacket->data[it].u = qFromBigEndian(curPacket->data[it].u);
        }

        if(!in.commitTransaction()) {
            //qDebug() << "Transaction fault! Status:" << in.status();
            return;
        }
#ifdef DEBUGLOG
        if(!keepAliveTimer.data()->isActive())
             keepAliveTimer.data()->start();
#endif
        curPacket->setTime();
        curPacket->setDate();

        qDebug() << curPacket->getDateTime().toString();

        qDebug() << this->bytesAvailable();

        nextBlockSize = 0;


        queReceivePackets.enqueue(curPacket);
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
    return (reconnectTimer.data()->isActive() || this->isOpen());
}
void PLCSocketClient::reconnect()
{
    qDebug() << "PLCSocketClient:: Try to reconnect number " << reconnectCount;
    this->connectToHost(_plcServer->address, _plcServer->port);
    reconnectDelay = GLOBAL::Fib(++reconnectCount)*1000;
}
void PLCSocketClient::isAlive()
{
    keepAliveTimer->stop();
    QScopedPointer<GlobalError> curError(new GlobalError());
    curError->setFirstItem(GlobalError::Debug);
    curError->setFrom(this->server()->address);
    curError->setSecondItem("Идет обмен данными на уровне сокета, формирование пакета");
    Logger::instance()->addEntryInFile(curError.data());
}
