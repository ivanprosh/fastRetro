#include "worker.h"
#include "global.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>

namespace {
    const QString _DBConnectionString("DRIVER={SQL Server};SERVER=%1;DATABASE=RUNTIME;UID=fastRetroUser;PWD=1;Trusted_Connection=no; WSID=.");
    //const QString _DBConnectionString("DRIVER={SQL Server};SERVER=%1;DATABASE=RUNTIME;Trusted_Connection=yes; WSID=.");
}


Worker::Worker(QObject *parent):QObject(parent),
    m_database(QSqlDatabase::addDatabase("QODBC")),
    curError(new GlobalError(GlobalError::None,"Ок"))
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

    //connect(&watcher, SIGNAL(directoryChanged(QString)),
    //        this, SLOT(directoryChanged(QString)));
}

bool Worker::executeQuery(const QString &query)
{
    //qDebug() << "Worker:: Send SQL query";
    QSqlQuery queryClass;

    if(queryClass.exec(query))
        return true;

    curError->setFirstItem(GlobalError::Historian);
    curError->setSecondItem(queryClass.lastError().text());
    emit errorChange(curError.data());

    qDebug() << queryClass.lastError().text();
    return false;
}

void Worker::setBackupFolder(const QString &path)
{
    backupFolder.setPath(path);
    watcher.addPath(path);
    scanfolder();
}

void Worker::scanfolder()
{
    if(!m_database.isValid() || !m_database.isOpen())
        return;//QTimer::singleShot(10000, this, SLOT(scanfolder()));

    QStringList filesNames = backupFolder.entryList(QStringList("*.fr"),QDir::Files,QDir::Name);

    checkedFiles.clear();
    queryStream.clear();

    qDebug() << "Worker::scanfolder: " << filesNames;

    if(filesNames.isEmpty())
        return;

    if(executeQuery(createTablePref)) {
        //QMutexLocker locker(&GLOBAL::globalMutex);

        foreach (QString name, filesNames) {

            if(executeQuery(bulkInsert.arg(backupFolder.path() + "/" + name))){
                //удаляем
                if(!QFile::remove(backupFolder.path() + "/" + name)){
                    curError->setFirstItem(GlobalError::System);
                    curError->setSecondItem(name + ": Ошибка удаления файла");
                    emit errorChange(curError.data());
                }
            } else {
                qDebug() << "Ошибка BULK " << bulkInsert.arg(backupFolder.path() + "/" + name);              
            }
            //обработанные файлы
            //checkedFiles << name;
        }
        //qDebug() << "Worker::queryPrepare: " << queryStream;
    }

    resultInsertQuery();
    //QTimer::singleShot(10000, this, SLOT(scanfolder()));
}

void Worker::resultInsertQuery()
{
    if(!executeQuery(resultInsert)) {
        qDebug() << "Finished NOT EXEC!";//что-то делаем
        //QTimer::singleShot(1000, this, SLOT(resultInsertQuery()));
    } else {
        qDebug() << "ALL EXEC!";
    }
}

void Worker::serverNameChanged(const QString &server)
{
    qDebug() << "Worker:: Init new connection to server: " << server;
    if(m_database.isValid() && m_database.isOpen())
        m_database.close();
    if(!server.isEmpty())
        m_database.setDatabaseName(QString(_DBConnectionString).arg(server));
    if(!m_database.open()) {
        curError->setFirstItem(GlobalError::Historian);
        curError->setSecondItem("Проверьте доступность указанного сервера Historian");
        emit errorChange(curError.data());
    }
    scanfolder();
}
