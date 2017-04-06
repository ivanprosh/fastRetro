#include <QtEndian>
#include <QDataStream>

#include "plcsocketclient.h"

static const int ConnectTimeout = 60 * 1000;
static const int SizeRecieveBuf = 1000;
static const int SizeCyclicQue = 300;
static const int MaxPacketSize = 1000;

int Packet::count = 0;

PLCSocketClient::PLCSocketClient(const QByteArray &Id, QObject *parent):
    QTcpSocket(parent),invalidateTimeout(false),nextBlockSizeFloat(0), queReceivePackets(SizeCyclicQue)
{
    //timeoutTimer = startTimer(ConnectTimeout);
    sockIdString = Id;

    connect(this, SIGNAL(readyRead()),SLOT(newDataAvailable()));
    connect(this, SIGNAL(connected()), SLOT(connectEstablished()));
    connect(this, SIGNAL(disconnected()), SLOT(connectionClosed()));

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

    if (nextBlockSizeUint == 0) {
        if (this->bytesAvailable() < sizeof(quint32))
            return;
        in >> nextBlockSizeUint;
        nextBlockSizeUint = qFromBigEndian(nextBlockSizeUint);
        qDebug() << "Size is " << nextBlockSizeFloat;
    }

    if (this->bytesAvailable() < (nextBlockSizeFloat - sizeof(quint32))){
        qDebug() << "low bytes available: " << this->bytesAvailable();
        return;
    }
    qDebug() << this->bytesAvailable();
    if(nextBlockSizeFloat < MaxPacketSize && nextBlockSizeFloat > 0) {
        //создаем структуру для хранения одного пакета данных
        QSharedPointer<Packet> curPacket(new Packet(nextBlockSizeFloat - sizeof(quint32)));

        //in.startTransaction();
        this->_plcServer->lastVisited = QDateTime::currentDateTime().toTime_t();

        //считывание потока в контейнер
        for(int it=0;it<(nextBlockSizeFloat-sizeof(quint32)-1);it++){
            in >> curPacket->data[it].u;
            curPacket->data[it].u = qFromBigEndian(curPacket->data[it].u);
        }

        //curPacket->setTime((int)curPacket->getValue(0),(int)curPacket->getValue(1),(int)curPacket->getValue(2),(int)(curPacket->getValue(3)/1000));
        curPacket->setTime();
        curPacket->setDate();
        /*
         qDebug() << curPacket->getParCount()<< ","
                  << curPacket->getCyclesCount() << " "
                  << curPacket->getDate().toString(Qt::ISODate) << curPacket->getTime().toString("hh:mm:ss.zzz");
        */
        nextBlockSizeFloat = 0;
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
void PLCSocketClient::connectionClosed()
{
    //if (nextBlockSize != 0xFFFF)
    //    emit connectionClosedByServer();
        //\! 1 сокет
        //_model->setData(_model->index(0,statusColumn),"Соединение закрыто сервером",statusRole);
    closeConnection();

}

void PLCSocketClient::connectEstablished()
{
    readAll();
    qDebug() << "PLCSocketClient:: id " << this->_plcServer->id << " Соединение установлено";
}
void PLCSocketClient::closeConnection()
{
    this->close();
    //searchButton->setEnabled(true);
    //stopButton->setEnabled(false);
    //progressBar->hide();
}
