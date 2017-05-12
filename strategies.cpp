#include <QDebug>
//#include <QSqlQuery>
//#include <QSqlError>
#include "strategies.h"
#include "global.h"
#include "logger.h"

namespace {
    const QString _DBConnectionString("DRIVER={SQL Server};SERVER=%1;DATABASE=RUNTIME;UID=fastRetroUser;PWD=1;Trusted_Connection=no; WSID=.");
    //const QString _DBConnectionString("DRIVER={SQL Server};SERVER=%1;DATABASE=RUNTIME;Trusted_Connection=yes; WSID=.");
    const QString createTablePref("IF OBJECT_ID('tempdb.dbo.#FastRetroBuffer', 'U') IS NOT NULL\n"
                                  "  DROP TABLE #FastRetroBuffer; \n"
                                  "CREATE TABLE #FastRetroBuffer\n"
                                  "(\n"
                                  "	DateTime datetime2(7) NOT NULL,  \n"
                                  "    TagName nvarchar(256) NOT NULL, \n"
                                  "    Value float\n"
                                  ");\n");

    const QString bulkInsert("BULK INSERT #FastRetroBuffer "
                             "FROM '%1'"
                             "WITH"
                             " ("
                             " FIELDTERMINATOR =', ',"
                             " ROWTERMINATOR = '\\n'"
                             " )");
    const QString resultInsert("INSERT INTO INSQL.Runtime.dbo.AnalogHistory (DateTime,TagName,Value) "
                               "SELECT DateTime,TagName,Value FROM #FastRetroBuffer");

}

void AbstractStrategy::setHistorianPath(const QString& path){
    qDebug() << "RegExp:: " << path;
    //QRegExp expr("\\\\(\\w*)(\\.*)?");
    QRegExp expr("\\\\\\\\(\\w*)(\\\\.*)?");
    if(!expr.exactMatch(path)){
        qDebug() << "New path: error";
        curError->setFirstItem(GlobalError::Historian);
        curError->setSecondItem("Некорректный путь до сервера Historian (проверьте синтаксис)");
        emit errorChange(curError.data());
        return;
    }
    if(!expr.cap(1).isEmpty()){
        HistServerName = expr.cap(1);
        if(!expr.cap(2).isEmpty())
            HistServerFastFolder.setPath("\\\\" + expr.cap(1) + expr.cap(2));
        reinit();
        qDebug() << "New path: cap(1) - " << expr.cap(1) << " HistServerFastFolder - " << HistServerFastFolder;
    } else {
        qDebug() << "New path: cap(1) - wrong";
    }
}
#ifdef FORWARD
Forward::Forward(QObject *parent):AbstractStrategy(parent),
    m_database(QSqlDatabase::addDatabase("QODBC"))
{
}
void Forward::reinit() {
    if(m_database.isValid() && m_database.isOpen())
        m_database.close();
    if(!HistServerName.isEmpty())
        m_database.setDatabaseName(QString(_DBConnectionString).arg(HistServerName));
    if(!m_database.open()) {
        curError->setFirstItem(GlobalError::Historian);
        curError->setSecondItem("Проверьте доступность указанного сервера Historian");
        emit errorChange(curError.data());
    }
    scanFolder();
}

bool Forward::executeQuery(const QString &query)
{
    QSqlQuery queryClass;

    if(queryClass.exec(query))
        return true;

    curError->setFirstItem(GlobalError::Historian);
    curError->setSecondItem(queryClass.lastError().text());
    emit errorChange(curError.data());

    return false;
}

void Forward::resultInsertQuery()
{
    if(!executeQuery(resultInsert)) {
        qDebug() << "Finished NOT EXEC!";//что-то делаем
        //QTimer::singleShot(1000, this, SLOT(resultInsertQuery()));
    } else {
        qDebug() << "ALL EXEC!";
    }
}

void Forward::scanFolder()
{
    if(!componentReady()){
        curError->setFirstItem(GlobalError::Configuration);
        curError->setSecondItem("Нет подключения к БД");
        emit errorChange(curError.data());
        return;
    }

    QStringList filesNames = backupFolder.entryList(QStringList("*.csv"),QDir::Files,QDir::Name);

    try {
    if(filesNames.isEmpty())
        return;

    if(executeQuery(createTablePref)) {

        foreach (QString name, filesNames) {

            if(executeQuery(QString(bulkInsert).arg(backupFolder.path() + "/" + name))){
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
    }

    catch (GlobalError error) {
        curError->setFirstItem(error.firstItem());
        curError->setSecondItem(error.secondItem());
        emit errorChange(curError.data());
    }
}
#endif
/*
 *
 */
Native::Native(QObject *parent):AbstractStrategy(parent)
{
    qDebug() << " Native()";
    connect(&watcher,SIGNAL(directoryChanged(QString)),
            this,SLOT(backupDirectoryChanged(QString)));

}

void Native::scanFolder()
{
    if(!componentReady()){
        curError->setFirstItem(GlobalError::Configuration);
        curError->setSecondItem("Не задан(не доступен) каталог копирования файлов на сервере Historian");
        emit errorChange(curError.data());
        return;
    }
    int errCount = 0;

    //чтобы не было копирования пустых файлов
    GLOBAL::globalMutex.lock();
        QStringList filesNames = backupFolder.entryList(QStringList("*.csv"),QDir::Files,QDir::Name);
    GLOBAL::globalMutex.unlock();

    if(filesNames.isEmpty())
        return;

    foreach (QString name, filesNames) {
        QFileInfo info(backupFolder.path() + "/" + name);
        //qDebug() << info.size();
        if(info.size() < 100){
            qDebug() << "File is busy: " << name;
            continue;
        }
        //if(!QFile::copy(backupFolder.path() + "/" + name,"\\\\WW_SERVER\\FastRetroBuf\\" + name)){
        if(!QFile::copy(backupFolder.path() + "/" + name,HistServerFastFolder.path() + "/" + name)){
            curError->setFirstItem(GlobalError::System);
            curError->setFrom(name);
            curError->setSecondItem("Ошибка копирования файла");
            emit errorChange(curError.data());
            errCount++;
        } else {
#ifdef DEBUGLOG
        curError->setFirstItem(GlobalError::Debug);
        curError->setFrom(name);
        curError->setSecondItem("Файл скопирован на сервер, размер " + QString::number(info.size()));
        emit errorChange(curError.data());;
#endif
            //для сличения
            //QFile::copy(backupFolder.path() + "/" + name, "/" + name );

            if(!QFile::remove(backupFolder.path() + "/" + name)) {
                curError->setFirstItem(GlobalError::System);
                curError->setFrom(name);
                curError->setSecondItem("Ошибка удаления файла");
                emit errorChange(curError.data());
                errCount++;
            }
        }
    }
    if(errCount > 50) {
        //очищаем каталог
        foreach (QString name, filesNames)
            QFile::remove(backupFolder.path() + "/" + name);
        /* ничего не делаем, чтобы при появлении сервера в сети скопировать сегменты
        curError->setFirstItem(GlobalError::Configuration);
        curError->setSecondItem("Проверьте права приложения на удаление/копирование файлов и доступность сервера Hist.");
        emit errorChange(curError.data());
        */
    }
}
void Native::setBackupPath(const QString &path)
{
    qDebug() << "Native:: Backup Path is " << path;
    QFileInfo backupDir(path);
    if(!backupDir.isDir()){
        curError->setFirstItem(GlobalError::Configuration);
        curError->setSecondItem("Не задан(не доступен) каталог локального хранения файлов");
        emit errorChange(curError.data());
        return;
    }
    AbstractStrategy::setBackupPath(path);
    watcher.addPath(path);
    reinit();
}

void Native::backupDirectoryChanged(QString)
{
    qDebug() << "Native:: New file detected";
    scanFolder();
}

void Native::reinit() {
    qDebug() << "Native:: reinit()";
    scanFolder();
}

bool Native::componentReady() {
    if(HistServerFastFolder.path().isEmpty())
        return false;
    QFileInfo destinationDir(HistServerFastFolder.path());

    return destinationDir.isDir();
}
