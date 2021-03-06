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

    void insertDataInStream(QSharedPointer<PLCServer> server, QSharedPointer<Packet> curPacket, QString& query);
    void initialize();
    void prepareQuery(QString&,int cycleIndex, int cycleStep, QDateTime curDateTime);
    //QMutex* mutex;

public slots:
    void newDataReceived();
    void queryResult(bool result);
    //void setDB(const QString& server);
    void setNewFilePath(const QString& filepath) {_filepath = filepath;}
    //void threadInitComplete();

private:
    QString generateFileName(const QDateTime& dt, const QString &abonent);
    void streamtoFile(const QString &fileName, const QString& stream, QString filepath);

    QHash<int,QStringList> PLCtoParNames;
    QString _filepath;
    QStringList queries;

    QString templateQuery;
    QString insertHistTableQuery;

    void errorHandler(GlobalError::ErrorRoles, const QString&, const int idfrom = 1000);
signals:
    void errorChange(GlobalError*);

};

#endif // DATAANALIZATOR_H
