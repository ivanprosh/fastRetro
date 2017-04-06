#include <QSettings>
#include <QSqlError>
#include "plcsocketclient.h"
#include "dataanalizator.h"
#include "globalerror.h"
#include "global.h"

Q_GLOBAL_STATIC(DataAnalizator, dataAnalizator)

static bool INIT_TRUE = false;

DataAnalizator* DataAnalizator::instance()
{
    return dataAnalizator();
}

void DataAnalizator::newDataReceived()
{
    PLCSocketClient* client = static_cast<PLCSocketClient*>(sender());

    if(client == Q_NULLPTR) return;

//    if(PLCtoParNames.size() < (client->getServer()->id))
//        qDebug() << "DataAnalizator::Unknown PLC";
    if(ignorePLClist.value(client->getServer()->id)) return;

    int it=client->queReceivePackets.size();

    if(!PLCtoParNames.contains(client->getServer()->id) || PLCtoParNames.value(client->getServer()->id).isEmpty()){
        errorHandler(GlobalError::Configuration,"Не найдены быстрые параметры абонента. Проверьте конф. файл",client->getServer()->id);
        return;
    }
    if(it>0 && PLCtoParNames.value(client->getServer()->id).size() < client->queReceivePackets.first()->getParCount()){
        errorHandler(GlobalError::Configuration,
                     PLCtoParNames.value(client->getServer()->id).first() + ": Проверьте идентичность количества параметров в контроллере и конф. файле", client->getServer()->id);
        return;
    }

    //если накопилось достаточно данных и успешно отправлен последний запрос
    if(it>50) {
        if(currentThread==Q_NULLPTR) {
            qDebug() << "New data received but currentThread is NULL!";
            return;
        }
        QString query;
        query.append(createTablePref);

        while(!client->queReceivePackets.isEmpty() && (it > 0)){
            insertDataInQuery(client->getServer(),client->queReceivePackets.head(), query);
            client->queReceivePackets.dequeue();
            it--;
        }
        query.append(insertHistTableQuery);
        //qDebug() << query;
        //currentThread->execute(query);
        //lastQuerySuccess = false;
    }

    //forbiddenReceive = false;
}

void DataAnalizator::queryResult(bool result)
{
    //lastQuerySuccess = result;
    qDebug() << "Query success is - " << result;
}

void DataAnalizator::prepareQuery(QString& _prepareQuery,int cycleIndex, int cycleStep, QDateTime curDateTime)
{
    QDateTime corectDateTime(curDateTime.addMSecs(cycleStep*cycleIndex));

    _prepareQuery = templateQuery.arg(corectDateTime.date().toString(Qt::ISODate))
                                 .arg(corectDateTime.time().toString("hh:mm:ss.zzz"));
}

void DataAnalizator::insertDataInQuery(QSharedPointer<PLCServer> server, QSharedPointer<Packet> curPacket, QString& query)
{
    int cycleIndex = 0;
    QString _prepareQuery;

    while(cycleIndex < curPacket->getCyclesCount()){
        int curPar = 0;
        prepareQuery(_prepareQuery,cycleIndex,server->cycleStep,curPacket->getDateTime());
        while (curPar < curPacket->getParCount()) {
            query.append(_prepareQuery.arg(PLCtoParNames.value(server->id).first() + "." + PLCtoParNames.value(server->id).at(curPar+1))
                         .arg(curPacket->getValue(Packet::startData + curPar + cycleIndex*curPacket->getParCount())));
            curPar++;
        }
        cycleIndex++;
    }

}

void DataAnalizator::initialize()
{
    if(!INIT_TRUE){
        qDebug() << "Start Thread!";
        //mutex = new QMutex();
        currentThread = new WorkThread();
        //mutex->lock();
        connect( currentThread, SIGNAL( queryFinished( bool ) ),
                 this, SLOT( queryResult(bool) ) );
        //connect( currentThread, SIGNAL( initFinished()),
        //         this, SLOT(threadInitComplete()));
        currentThread->start();
        //mutex->unlock();
        INIT_TRUE = true;
    }

    createTablePref =
    "DECLARE @FastTableVar table( \n"
    "    DateTime datetime2(7) NOT NULL, \n"
    "    TagName nvarchar(256) NOT NULL, \n"
    "    Value float,  \n"
    "    Quality tinyint, \n"
    "    QualityDetail int, \n"
    "    wwTagKey int);\n";

    templateQuery =
    "INSERT @FastTableVar (DateTime,TagName,Value) "
    "VALUES ('%1 %2', '%3', %4) \n";

    insertHistTableQuery =
    "INSERT INTO INSQL.Runtime.dbo.AnalogHistory (DateTime,TagName,Value) "
    "SELECT DateTime,TagName,Value FROM @FastTableVar \n";
}

//Чтение из файла
QString DataAnalizator::rfile(const QString& name)
{
    QSettings settings(name, QSettings::IniFormat);
    QStringList Names;

    //общие
    int index(0);
    QStringList keys = settings.childGroups();

    qDebug() << "keys!" << keys;

    foreach (QString key, keys) {

        settings.beginGroup(key);

        const QStringList childKeys = settings.childKeys();

        if(childKeys.empty()) return QString("Формат файла некорректный: в группе [Names] необходимо перечислить хранимые параметры через ,");

        PLCtoParNames[index].append(key);
        PLCtoParNames[index].append(settings.value("Names").toStringList());

        if(Names.size() > MAX_PAR_COUNT) return QString("Параметров должно быть не более " + QString::number(MAX_PAR_COUNT));

        settings.endGroup();

        qDebug() << index << ":" << PLCtoParNames.value(index);

        index++;
    }


    return QString();
}

void DataAnalizator::setDB(const QString &server)
{
    qDebug() << "DataAnalizator::Server change name: " << server;

    QMutexLocker locker(&GLOBAL::globalMutex);

    if(currentThread==Q_NULLPTR) {
        qDebug() << "currentThread is NULL!";
        return;
    }

    if(currentThread->getWorker()->getDB().isValid() && currentThread->getWorker()->getDB().isOpen())
        currentThread->getWorker()->getDB().close();

    QString stringConnection="DRIVER={SQL Server};SERVER=%1;DATABASE=RUNTIME;UID=wwAdmin;PWD=wwAdmin;Trusted_Connection=no; WSID=.";
    currentThread->setConnection(stringConnection.arg(server));

    if(currentThread->getWorker()->getDB().open()){
        qDebug() << "DataAnalizator::DB open!";

        return;
    }
    //mutex->unlock();
    QScopedPointer<GlobalError> CurError(new GlobalError(GlobalError::Historian,
                                                         getlastErrorDB()));
    emit errorChange(CurError.data());
    //emit connectError(getlastErrorDB());
    //return false;
}

void DataAnalizator::errorHandler(GlobalError::ErrorRoles role, const QString &text, const int idfrom = 1000)
{
    QScopedPointer<GlobalError> CurError(new GlobalError(role,text));
    CurError->setIdFrom(idfrom);

    //if(!ignorePLClist.value(idfrom))
    emit errorChange(CurError.data());

    if(role==GlobalError::Configuration) {
       if(idfrom!=1000) ignorePLClist[idfrom] = true;
    }
}

QString DataAnalizator::getlastErrorDB()
{
    qDebug() << " DataAnalizator:: last DB err: " <<  currentThread->getWorker()->getDB().lastError().text();
    //if(currentThread->getWorker()->getDB().isValid())
        return currentThread->getWorker()->getDB().lastError().text();
    //return QString();
}
