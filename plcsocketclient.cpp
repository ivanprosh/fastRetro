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

//константы для настройки таймаутов, ограничения размеров
namespace {
    static const int ConnectTimeout = 60 * 1000;
    static const int SizeRecieveBuf = 1000;
    static const int SizeCyclicQue = 300;
    static const int MaxPacketSize = 10000;
    //const int reconnectDelay = 5000;
}

PLCSocketClient::PLCSocketClient(const QByteArray &Id, QObject *parent):
    QTcpSocket(parent),
    invalidateTimeout(false),
    nextBlockSize(0), reconnectDelay(1000),reconnectCount(0),
    queReceivePackets(SizeCyclicQue)
{
    //timeoutTimer = startTimer(ConnectTimeout);
    sockIdString = Id;
    //включаем опцию KeepAlive для контроля обрыва соединения в случае молчания в линии
    //по дефолту, если линия оборвется, то мы об этом не узнаем, KeepAlive пакет - диагностический пакет, который отсылается по линии для контроля обрыва
    setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    //таймер переподключения
    reconnectTimer = QSharedPointer<QTimer>(new QTimer());
    reconnectTimer->setInterval(reconnectDelay);
    reconnectTimer->setSingleShot(true);
    //используется для вывода отладочной информации в лог о состоянии соединения
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
//есть новая порция данных
void PLCSocketClient::newDataAvailable()
{
    QDataStream in(this);
    invalidateTimeout = true;
    //если обнулем nextBlockSize - значит, ожидаем новый пакет
    if (nextBlockSize == 0) {
        if (this->bytesAvailable() < sizeof(quint32))
            return;
        quint32 size;
        in >> size;
        //здесь странный момент - Hex Generator говорит, что данные передаются в формате LittleEndian (перехват через Wireshark),
        //однако на уровне Qt они корректно выводятся при конвертации через qFromBigEndian
        nextBlockSize = qFromBigEndian(size);
        if(nextBlockSize != 424)
            qDebug() << "Size is " << nextBlockSize;
    }

    if (nextBlockSize < 0 || nextBlockSize > MaxPacketSize) {
        setErrorString("Проверьте формат протокола обмена. Некорректный размер пакета");
        emit error(QAbstractSocket::SslInvalidUserDataError);
        return;
    }
    //ждем пока накопится достаточное количество данных (полноценный пакет)
    if (this->bytesAvailable() < (nextBlockSize - sizeof(quint32))){
        qDebug() << "low bytes available: " << this->bytesAvailable();
        return;
    }
    //не проверено, встроено для ликвидации последствий некорретного протокола сос стороны сервера, для исключения вылета всего приложения
    try{

        //создаем структуру для хранения одного пакета данных
        QSharedPointer<Packet> curPacket(new Packet(nextBlockSize));
        //инфраструктура транзакций позволяет откатить полностью операцию с восстановлением потока, если что-то пойдет не так
        //пока не выполнится commitTransaction()
        in.startTransaction();

        this->_plcServer->lastVisited = QDateTime::currentDateTime().toTime_t();
        //резервный инт для выравнивания до четного числа INT'ов. Проблема AllenBradley, так как у них если после структуры INT'ов идет массив DINT,
        //кол-во int'ов выравнивается до четного числа
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

        //qDebug() << QDateTime::currentDateTime().time();
        //qDebug() << "Packet:: parCount, Cycles " << curPacket->ParCount << " " << curPacket->CyclesCount;
        //считывание потока в контейнер
        for(int it=0;it<curPacket->getDataSize();it++){
            in >> curPacket->data[it].u;
            curPacket->data[it].u = qFromBigEndian(curPacket->data[it].u);
        }
        if(!in.commitTransaction()) {
            return;
        }


#ifdef DEBUGLOG
        if(!keepAliveTimer.data()->isActive())
             keepAliveTimer.data()->start();
#endif
        curPacket->setTime();
        curPacket->setDate();
        //qDebug() << curPacket->getDateTime().toString();
        qDebug() << this->bytesAvailable();
        nextBlockSize = 0;
        queReceivePackets.enqueue(curPacket);
        emit newDataReceived();
        //повторяем обработку, если в буфере есть еще данные
        if(this->bytesAvailable()>0)
            newDataAvailable();
    }
    catch(...) {
        setErrorString("Проверьте формат протокола обмена. Неизвестная ошибка");
        emit error(QAbstractSocket::SslInvalidUserDataError);
        return;
    }
}

void PLCSocketClient::connectEstablished()
{
    //активируем keep-alive. Используем структуру из WinSock, так как необходимо откорректировать
    //стандартный интервал отправки пакетов KeepAlive. По умолчанию в Windows отправка диагностических
    //пакетов контроля состояния линии начнется лишь через два часа после простоя обмена
    tcp_keepalive vals;
    vals.onoff             = 1;    // non-zero means "enable"
    vals.keepalivetime     = 5000;   // через какое время простоя начать слать пакеты
    vals.keepaliveinterval = 1000;  // интервал отправки

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
    //новая задержка высчитывается по ряду Фибоначчи
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
