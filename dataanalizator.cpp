#ifdef FORWARD
    #include <QSqlError>
#endif
#include <QFile>
#include <QFileInfo>
#include <QSettings>
#include "plcsocketclient.h"
#include "dataanalizator.h"
#include "globalerror.h"
#include "global.h"

Q_GLOBAL_STATIC(DataAnalizator, dataAnalizator)

static bool INIT_TRUE = false;

namespace {
    const QString FileHeaderData = QString("ASCII\n,\nUser,1,Server Local,10,2\n").toUtf8();
}

DataAnalizator* DataAnalizator::instance()
{
    return dataAnalizator();
}

DataAnalizator::DataAnalizator():_segmentInterval(1){}

void DataAnalizator::newDataReceived()
{
    PLCSocketClient* client = static_cast<PLCSocketClient*>(sender());

    if(client == Q_NULLPTR) return;

    if(ignorePLClist.value(client->getServer()->id)) return;

    int it=client->queReceivePackets.size();

    if(PLCtoParNames.value(client->getServer()->id).isEmpty()){
        errorHandler(GlobalError::Configuration,
                     "Проверьте формат описания конф. файла signals.ini. Не найдены параметры");
        return;
    }

    if(!PLCtoParNames.contains(client->getServer()->id)){
        errorHandler(GlobalError::Configuration,
                     "Не найдены быстрые параметры абонента. Проверьте конф. файл",
                     PLCtoParNames.value(client->getServer()->id).first(),
                     client->getServer()->id);
        return;
    }
    if(it>0 && PLCtoParNames.value(client->getServer()->id).size() != client->queReceivePackets.first()->getParCount() + 1){
        errorHandler(GlobalError::Configuration,
                     "Проверьте идентичность количества параметров в контроллере и конф. файле (значение параметра ParCount в ПЛК)",
                     PLCtoParNames.value(client->getServer()->id).first(),
                     client->getServer()->id);
        return;
    }

    //если накопилось достаточно данных
    if(it>_segmentInterval*10) {

        qDebug() << "New data available in DataAnalizator";
        QString stream;

        if(_isRedundant){
            if(!redundancyHandler())
                return;
        }
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
    }

}

void DataAnalizator::queryResult(bool result)
{
    qDebug() << "Query success is - " << result;
}

void DataAnalizator::correctDTime(QString& _prepareDTime,int cycleIndex, int cycleStep, QDateTime curDateTime)
{
    QDateTime corectDateTime(curDateTime.addMSecs(cycleStep*cycleIndex));

    _prepareDTime = QString("%1,%2").arg(corectDateTime.date().toString("yyyy/MM/dd"))
                                    .arg(corectDateTime.time().toString("hh:mm:ss.zzz"));

    if(_curTimeZone != 0)
        corectDateTime = corectDateTime.addSecs(_curTimeZone*3600);

    _prepareDTime = QString("%1,%2").arg(corectDateTime.date().toString("yyyy/MM/dd"))
                                    .arg(corectDateTime.time().toString("hh:mm:ss.zzz"));
}

bool DataAnalizator::redundancyHandler()
{
    QFileInfo redundantPath(_redundantFilePath);
    if(!redundantPath.isFile()){
        errorHandler(GlobalError::Configuration,
                     "Проверьте путь до файла резервирования");
        return false;
    }
    QFile rFile(_redundantFilePath);
    QTextStream in(&rFile);

    if (!rFile.open(QIODevice::ReadOnly | QIODevice::Text)){
        errorHandler(GlobalError::System,
                     "Ошибка открытия файла резервирования.");
    } else {
        char master;
        in >> master;
        //qDebug() << "Symbol is " << master;
        if(QString(master).toInt() == 1){
            _isMaster = true;
        }
        else {
            _isMaster = false;
            return false;
        }
        rFile.close();
    }
    return true;
}

void DataAnalizator::insertDataInStream(QSharedPointer<PLCServer> server, QSharedPointer<Packet> curPacket, QString& stream)
{
    int cycleIndex = 0;
    QString _prepareDTime;

    while(cycleIndex < curPacket->getCyclesCount()){
        int curPar = 0;
        correctDTime(_prepareDTime,cycleIndex,server->cycleStep,curPacket->getDateTime());
        while (curPar < curPacket->getParCount()) {
            QString TagName = PLCtoParNames.value(server->id).first() + "." + PLCtoParNames.value(server->id).at(curPar+1);
            float Value = curPacket->getValue(Packet::startData + curPar + cycleIndex*curPacket->getParCount());
            stream.append(QString("%1,%2,%3,%4,%5,192\n").arg(TagName).arg(QString::number(0)).arg(_prepareDTime).arg(QString::number(0)).arg(Value));
            //stream.append(_prepareQuery.arg(PLCtoParNames.value(server->id).first() + "." + PLCtoParNames.value(server->id).at(curPar+1))
            //             .arg(curPacket->getValue(Packet::startData + curPar + cycleIndex*curPacket->getParCount())));
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
//    templateQuery =
//    "%1,%2,%3,%4,%5\n";

    insertHistTableQuery =
    "INSERT INTO INSQL.Runtime.dbo.AnalogHistory (DateTime,TagName,Value) "
    "SELECT DateTime,TagName,Value FROM @FastTableVar \n";
}

//Чтение из файла
QString DataAnalizator::rfile(const QString& name)
{
    QSettings settings(name, QSettings::IniFormat);

    if(settings.status()!=QSettings::NoError){
        errorHandler(GlobalError::Configuration,
                     "Проблемы с открытием конф. файла!");
        return QString("Err");
    }
    QStringList Names;
    //общие
    int index(0);
    QStringList keys = settings.childGroups();

    foreach (QString key, keys) {

        settings.beginGroup(key);
        const QStringList childKeys = settings.childKeys();

        if(childKeys.empty())
            return QString("Формат файла некорректный: в группе [Names] необходимо перечислить хранимые параметры через ,");

        PLCtoParNames[index].append(key);
        PLCtoParNames[index].append(settings.value("Names").toStringList());

        if(Names.size() > MAX_PAR_COUNT) return QString("Параметров должно быть не более " + QString::number(MAX_PAR_COUNT));

        settings.endGroup();
        index++;

    }

    return QString("");
}

QString DataAnalizator::generateFileName(const QDateTime &dt, const QString& abonent)
{
    QString pattern = "%1_%2_%3%4";
    QDateTime corectDateTime = dt;
    if(_curTimeZone != 0)
        corectDateTime = corectDateTime.addSecs(_curTimeZone*3600);
    return pattern.arg(abonent).arg(corectDateTime.date().toString("yyyyMMdd")).arg(corectDateTime.time().toString("hhmmss"));
}

void DataAnalizator::streamtoFile(const QString &fileName,const QString& stream, QString filepath)
{
    //stream = GPA1_20170405_120522%1
    if(filepath.isEmpty()) {
        errorHandler(GlobalError::Configuration,
                     "Каталог хранения временных файлов не задан");
        return;
    }

    GLOBAL::globalMutex.lock();

    //QScopedPointer<QFile> outputFile(new QFile(filepath.append("/"+fileName +".csv")));
    QScopedPointer<QFile> outputFile(new QFile(filepath.append("\\"+fileName +".csv")));
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

    out << FileHeaderData;
    out << stream;

    outputFile->close();
    GLOBAL::globalMutex.unlock();
}

void DataAnalizator::errorHandler(GlobalError::ErrorRoles role, const QString &text, const QString& from,int _idFrom)
{
    QScopedPointer<GlobalError> CurError(new GlobalError(role,text));
    CurError->setFrom(from);
    CurError->setPlcIdFrom(_idFrom);

    emit errorChange(CurError.data());

}
