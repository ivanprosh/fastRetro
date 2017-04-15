#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QSqlDatabase>
#include <QDir>
#include <QFileSystemWatcher>

#include "globalerror.h"

class QFileSystemWatcher;

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker( QObject* parent = 0);
    ~Worker(){}
    QSqlDatabase& getDB() {return m_database;}
    //void addDatabase(const QString& driver="QODBC");

public slots:
    bool executeQuery( const QString& query );
    void setBackupFolder( const QString& );
    //void directoryChanged( QString );
    void scanfolder();
    void resultInsertQuery();
    void serverNameChanged(const QString&);
signals:
    //void results( const QList<QSqlRecord>& records );
    void results( bool success );
    void errorChange(GlobalError*);

private:

    QSqlDatabase m_database;
    QDir backupFolder;
    QString createTablePref, bulkInsert, resultInsert;
    QStringList checkedFiles;
    QString queryStream;
    QFileSystemWatcher watcher;
    QSharedPointer<GlobalError> curError;
};

#endif // WORKER_H
