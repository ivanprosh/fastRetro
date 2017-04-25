#ifndef DATAANALIZATOR_H
#define DATAANALIZATOR_H

#include <QObject>
#include <QDebug>
//#include "workthread.h"
#include "globalerror.h"

class Packet;
class PLCServer;


class DataAnalizator: public QObject//, private Ui::TripPlanner
{
    Q_OBJECT

public:
    static DataAnalizator* instance();
    //void startWorkerThread() {currentThread->start();}
    DataAnalizator();
    QString rfile(const QString &name);
    QHash<int,bool> ignorePLClist;

    void insertDataInStream(QSharedPointer<PLCServer> server, QSharedPointer<Packet> curPacket, QString& query);
    void initialize();
    void correctDTime(QString&,int cycleIndex, int cycleStep, QDateTime curDateTime);
    bool redundancyHandler();
    //QMutex* mutex;

public slots:
    void newDataReceived();
    void queryResult(bool result);
    //void setDB(const QString& server);
    void setNewFilePath(const QString& filepath) {_filepath = filepath;}
    void timeZoneChanged(const QString& timeZone) {_curTimeZone = timeZone.toInt();}
    void segmentIntervalChanged(int interval) {_segmentInterval = interval;}
    void setRedundantOption(bool val){_isRedundant = val;}
    void setRedundancyFilePath(const QString& val){_redundantFilePath = val;}
    //void threadInitComplete();

private:
    QString generateFileName(const QDateTime& dt, const QString &abonent);
    void streamtoFile(const QString &fileName, const QString& stream, QString filepath);

    QHash<int,QStringList> PLCtoParNames;
    QString _filepath,
            _redundantFilePath;
    QStringList queries;

    QString templateQuery;
    QString insertHistTableQuery;
    qint64 _curTimeZone;
    int _segmentInterval;
    bool _isRedundant;
    bool _isMaster;

    void errorHandler(GlobalError::ErrorRoles, const QString&, const QString &from = QString(), int _idFrom = -1);
signals:
    void errorChange(GlobalError*);

};

#endif // DATAANALIZATOR_H
