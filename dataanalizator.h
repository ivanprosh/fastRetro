#ifndef DATAANALIZATOR_H
#define DATAANALIZATOR_H

#include <QObject>
#include <QDebug>
#include <QSqlDatabase>
#include "workthread.h"
#include "globalerror.h"

class Packet;
class PLCServer;


class DataAnalizator: public QObject//, private Ui::TripPlanner
{
    Q_OBJECT

public:
    static DataAnalizator* instance();
    //void startWorkerThread() {currentThread->start();}

    QString rfile(const QString &name);
    QHash<int,bool> ignorePLClist;
    QString getlastErrorDB();

    void insertDataInQuery(QSharedPointer<PLCServer> server, QSharedPointer<Packet> curPacket, QString& query);
    void initialize();
    void prepareQuery(QString&,int cycleIndex, int cycleStep, QDateTime curDateTime);
    //QMutex* mutex;

public slots:
    void newDataReceived();
    void queryResult(bool result);
    void setDB(const QString& server);
    //void threadInitComplete();

private:
    QHash<int,QStringList> PLCtoParNames;
    QStringList queries;
    QString createTablePref;
    QString templateQuery;
    QString insertHistTableQuery;
    //QSqlDatabase db;
    WorkThread* currentThread;
//    bool lastQuerySuccess;
//    bool forbiddenReceive;
    //DataAnalizator(const DataAnalizator&);
    //DataAnalizator();
    void errorHandler(GlobalError::ErrorRoles, const QString&, const int idfrom);
signals:
    void errorChange(GlobalError*);

};

#endif // DATAANALIZATOR_H
