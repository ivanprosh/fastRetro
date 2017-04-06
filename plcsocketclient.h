#ifndef PLCSOCKETCLIENT_H
#define PLCSOCKETCLIENT_H

#include <QHostAddress>
#include <QTcpSocket>
#include <QSharedPointer>
#include <QTime>
#include <QQueue>

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
    const int size;
public:
    enum index {Year=0,Month,Day,Hour=3,Minute,Second,MicroSecond,ParCount = 7, CyclesCount, startData};
    static int count;
    QDateTime datetime;
    //QDate date;
    Packet(const int _size):size(_size){
        count++;
        data= new contain[_size];
    }
    ~Packet() {
        //qDebug() << "~Packet()";
        delete[] data;
    }
    static void initCount(int _count){count = _count;}
    contain* getData() const {return data;}

    QTime getTime() const{return datetime.time();}
    void setTime() {
        datetime.setTime(QTime((int)getValue(Hour),(int)getValue(Minute),(int)getValue(index::Second),(int)getValue(index::MicroSecond)/1000));
    }

    QDate getDate() const{return datetime.date();}
    void setDate() {
        datetime.setDate(QDate((int)getValue(index::Year),(int)getValue(index::Month),(int)getValue(index::Day)));
    }

    QDateTime getDateTime() const {return datetime;}

    int getParCount() const {return (int)getValue(index::ParCount);}
    int getCyclesCount() const {return (int)getValue(index::CyclesCount);}
    float getValue(int index) const { return (index<size) ?  data[index].f : float(0);
    }
};

//Кольцевой буфер на основе стандартной очереди
template <class T>
class CQueue : public QQueue <T> {
 //как наследоваться от стандартного контейнера
 //для любого типа данных T
 private:
  int count; //предельный размер
 public:
  inline CQueue (int cnt) : QQueue<T>(),count(cnt) {
   //конструктор
  }
  inline void enqueue (const T &t) {
   //перегрузка метода постановки в очередь
   if (count==QQueue<T>::count()) {
    QQueue<T>::dequeue();
    //удалить старейший элемент по заполнении
   }
   QQueue<T>::enqueue(t);
  }
};

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

    QSharedPointer<PLCServer> server() const;
    CQueue<QSharedPointer<Packet> > queReceivePackets;

signals:
    void connectionClosedByServer();
    void newDataReceived();

public slots:
    void connectionClosed();
    void connectEstablished();
    void newDataAvailable();
    void closeConnection();

private:
    // Timeout handling
    int timeoutTimer;
    bool invalidateTimeout;
    // Data waiting to be read/written
    QByteArray incomingBuffer;
    QByteArray outgoingBuffer;
    //quint32 nextBlockSize;
    union {
        float nextBlockSizeFloat;
        quint32 nextBlockSizeUint;
    };
    //+
    QSharedPointer<PLCServer> _plcServer;
    QByteArray sockIdString;

};

#endif // PLCSOCKETCLIENT_H
