#ifndef PLCSOCKETCLIENT_H
#define PLCSOCKETCLIENT_H

#include <QHostAddress>
#include <QTcpSocket>
#include <QSharedPointer>
#include <QTime>
#include <QTimer>

#include "circlequeue.h"

class QByteArray;

class PLCServer {
public:
    QString address;
    quint16 port;
    int id;
    int cycleStep;
    bool interesting;
    bool seed;
    uint lastVisited;
    uint connectStart;
    uint connectTime;
    //QBitArray pieces;
    //int numCompletedPieces;
    void setId(int _id){
        id = _id;
    }

    void setAddress(QString addrString){
        //id = curRow;
        address = addrString.left(addrString.indexOf(":"));
        port =  addrString.right(addrString.size()-addrString.indexOf(":")-1).toUInt();
    }

    inline bool operator==(const PLCServer &other)
    {
        return port == other.port
            && address == other.address;
    }
};

class Packet{
    friend class PLCSocketClient;
    struct contain{
        union {
            float f;
            quint32 u;
        };
        //float operator [](int i){return f;}
    };
    contain* data;
    int dataSize;
public:
    const int headerParCount = 11;
    //static int count;
    quint16 Year,
            Month,
            Day,
            Hour,
            Minute,
            Second,
            MSecond,
            ParCount,
            CyclesCount,
            Reserv; //необходим из-за выравнивания структур в AB

    QDateTime datetime;
    //QDate date;
    Packet(const int _size){
        //размер буфера данных (за вычетом размера служебных параметров)
        dataSize = (_size - headerParCount*sizeof(quint16))/sizeof(quint32);
        //count++;
        data= new contain[dataSize];
    }
    ~Packet() {
        //qDebug() << "~Packet()";
        delete[] data;
    }
    //static void initCount(int _count){count = _count;}
    contain* getData() const {return data;}

    QTime getTime() const{return datetime.time();}
    void setTime() {
        //datetime.setTime(QTime((int)getValue(Hour),(int)getValue(Minute),(int)getValue(index::Second),(int)getValue(index::MicroSecond)/1000));
        datetime.setTime(QTime(Hour,Minute,Second,MSecond));
    }

    QDate getDate() const{return datetime.date();}
    void setDate() {
        datetime.setDate(QDate(Year,Month,Day));
    }

    QDateTime getDateTime() const {return datetime;}

    int getParCount() const {return ParCount;}
    int getDataSize() const {return dataSize;}
    int getCyclesCount() const {return CyclesCount;}
    float getValue(int index) const { return (index<dataSize) ?  data[index].f : float(0);
    }
};
/*
struct _tcp_keepalive{
    u_long onoff;
    u_long keepalivetime;
    u_long keepaliveinterval;
};
*/
class PLCSocketClient : public QTcpSocket
{
    Q_OBJECT

public:

    //+
    explicit PLCSocketClient(const QByteArray &peerId, QObject *parent = 0);
    virtual ~PLCSocketClient();

    void initialize(const QByteArray &infoHash, int pieceCount);
    QSharedPointer<PLCServer> getServer();
    void setServer(const QSharedPointer<PLCServer> &peer);
    void startReconnectTimer();
    void stopReconnectTimer();
    bool isActive();

    QSharedPointer<PLCServer> server() const;
    CQueue<QSharedPointer<Packet> > queReceivePackets;

signals:
    void connectionClosedByServer();
    void newDataReceived();

public slots:
    void connectEstablished();
    void newDataAvailable();
    void closeConnection();
    void reconnect();
    void isAlive();

private:
    // Timeout handling
    //int timeoutTimer;
    int reconnectDelay;
    quint32 reconnectCount;
    bool invalidateTimeout;
    //keep-alive
    //_tcp_keepalive vals;

    // Data waiting to be read/written
    QByteArray incomingBuffer;
    QByteArray outgoingBuffer;
    quint16 nextBlockSize;

    QSharedPointer<PLCServer> _plcServer;
    QByteArray sockIdString;
    QSharedPointer<QTimer> reconnectTimer,keepAliveTimer;

};

#endif // PLCSOCKETCLIENT_H
