#include "worker.h"
#include "global.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

Worker::Worker(QObject *parent):QObject(parent),m_database(QSqlDatabase::addDatabase("QODBC"))
{
    createTablePref =
    "IF OBJECT_ID('tempdb.dbo.#FastRetroBuffer', 'U') IS NOT NULL\n"
    "  DROP TABLE #FastRetroBuffer; \n"
    "CREATE TABLE #FastRetroBuffer\n"
    "(\n"
    "	DateTime datetime2(7) NOT NULL,  \n"
    "    TagName nvarchar(256) NOT NULL, \n"
    "    Value float\n"
    ");\n";

    bulkInsert =
    "BULK INSERT #FastRetroBuffer "
    "FROM '%1'"
    "WITH"
    " ("
    " FIELDTERMINATOR =', ',"
    " ROWTERMINATOR = '\\n'"
    " )";

    resultInsert =
    "INSERT INTO INSQL.Runtime.dbo.AnalogHistory (DateTime,TagName,Value) "
    "SELECT DateTime,TagName,Value FROM #FastRetroBuffer";
}

bool Worker::executeQuery(const QString &query)
{
    //qDebug() << "Worker:: Send SQL query";
    QSqlQuery queryClass;

    if(queryClass.exec(query))
        return true;

    QScopedPointer<GlobalError> CurError(new GlobalError(GlobalError::Historian,
                                         queryClass.lastError().text()));
    emit errorChange(CurError.data());

    qDebug() << queryClass.lastError().text();
    return false;
}

void Worker::setBackupFolder(const QString &path)
{
    backupFolder.setPath(path);
    scanfolder();
}

void Worker::scanfolder()
{
    QStringList filesNames = backupFolder.entryList(QStringList("*.txt"),QDir::Files,QDir::Name);
    checkedFiles.clear();
    queryStream.clear();

    qDebug() << "Worker::scanfolder: " << filesNames;

    if(executeQuery(createTablePref)) {
        //QMutexLocker locker(&GLOBAL::globalMutex);

        foreach (QString name, filesNames) {

            if(executeQuery(bulkInsert.arg(backupFolder.path() + "/" + name))){
                //удаляем
                if(!QFile::remove(name)){
                    QScopedPointer<GlobalError> CurError(new GlobalError(GlobalError::System,
                                                         name + ": Ошибка удаления файла"));
                    emit errorChange(CurError.data());
                }
            } else {
                qDebug() << "Ошибка BULK " << bulkInsert.arg(backupFolder.path() + "/" + name);
            }
            //обработанные файлы
            //checkedFiles << name;
        }
        //qDebug() << "Worker::queryPrepare: " << queryStream;
    }
    //удаляем
//    QMutexLocker locker(&GLOBAL::globalMutex);
//    if(executeQuery(queryStream)){
//        foreach (QString name, checkedFiles) {
//            //QFile inputFile(name);
//            if(!QFile::remove(name)){
//                QScopedPointer<GlobalError> CurError(new GlobalError(GlobalError::System,
//                                                     name + ": Ошибка удаления файла"));
//                emit errorChange(CurError.data());
//            }
//        }
//    }
    if(!executeQuery(resultInsert)) {
        qDebug() << "Finished NOT EXEC!";//что-то делаем
    } else {
        qDebug() << "ALL EXEC!";
    }
}
