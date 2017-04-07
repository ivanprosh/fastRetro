#include <QSettings>
#include <QSqlError>
#include <QFile>

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

    //если накопилось достаточно данных
    if(it>50) {

        QString stream;
        //query.append(createTablePref);
        QString Filename = generateFileName(client->queReceivePackets.head()->getDateTime(),
                                            PLCtoParNames.value(client->getServer()->id).first());

        while(!client->queReceivePackets.isEmpty() && (it > 0)){
            insertDataInStream(client->getServer(),client->queReceivePackets.head(), stream);
            client->queReceivePackets.dequeue();
            it--;
        }
        //запись в файл
        streamtoFile(Filename,stream,_filepath);
        //query.append(insertHistTableQuery);
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

void DataAnalizator::insertDataInStream(QSharedPointer<PLCServer> server, QSharedPointer<Packet> curPacket, QString& stream)
{
    int cycleIndex = 0;
    QString _prepareQuery;

    while(cycleIndex < curPacket->getCyclesCount()){
        int curPar = 0;
        prepareQuery(_prepareQuery,cycleIndex,server->cycleStep,curPacket->getDateTime());
        while (curPar < curPacket->getParCount()) {
            stream.append(_prepareQuery.arg(PLCtoParNames.value(server->id).first() + "." + PLCtoParNames.value(server->id).at(curPar+1))
                         .arg(curPacket->getValue(Packet::startData + curPar + cycleIndex*curPacket->getParCount())));
            curPar++;
        }
        cycleIndex++;
    }

}

void DataAnalizator::initialize()
{
/*
    templateQuery =
    "INSERT @FastTableVar (DateTime,TagName,Value) "
    "VALUES ('%1 %2', '%3', %4) \n";
*/
    templateQuery =
    "%1 %2, %3, %4\n";

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

    foreach (QString key, keys) {

        settings.beginGroup(key);
        const QStringList childKeys = settings.childKeys();
        if(childKeys.empty()) return QString("Формат файла некорректный: в группе [Names] необходимо перечислить хранимые параметры через ,");

        PLCtoParNames[index].append(key);
        PLCtoParNames[index].append(settings.value("Names").toStringList());

        if(Names.size() > MAX_PAR_COUNT) return QString("Параметров должно быть не более " + QString::number(MAX_PAR_COUNT));

        settings.endGroup();
        //qDebug() << index << ":" << PLCtoParNames.value(index);
        index++;
    }

    return QString();
}

QString DataAnalizator::generateFileName(const QDateTime &dt, const QString& abonent)
{
    QString pattern = "%1_%2_%3%4";
    return pattern.arg(abonent).arg(dt.date().toString("yyyyMMdd")).arg(dt.time().toString("hhmmss"));
}

void DataAnalizator::streamtoFile(const QString &fileName,const QString& stream, QString filepath)
{
    //stream = GPA1_20170405_120522%1
    if(filepath.isEmpty()) {
        errorHandler(GlobalError::Configuration,
                     "Каталог хранения временных файлов не задан");
        return;
    }

    //QMutexLocker locker(&GLOBAL::globalMutex);

    QScopedPointer<QFile> outputFile(new QFile(filepath.append("/"+fileName +".txt")));
    QTextStream out(outputFile.data());

    int ver = 1;
    if(outputFile->exists())
        while(outputFile->exists()){
            outputFile->setFileName(outputFile->fileName().arg("_v"+QString::number(ver++)));
        }
    else
        outputFile->setFileName(outputFile->fileName().arg(""));

    if (!outputFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        errorHandler(GlobalError::System,
                     "Невозможно открыть файл " + outputFile->fileName());
        return;
    }

    out << stream;

    outputFile->close();
}

void DataAnalizator::errorHandler(GlobalError::ErrorRoles role, const QString &text, const int idfrom)
{
    QScopedPointer<GlobalError> CurError(new GlobalError(role,text));
    CurError->setIdFrom(idfrom);

    //if(!ignorePLClist.value(idfrom))
    emit errorChange(CurError.data());

    if(role==GlobalError::Configuration) {
       if(idfrom!=1000) ignorePLClist[idfrom] = true;
    }
}
