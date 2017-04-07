#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QSqlDatabase>
#include <QDir>

#include "globalerror.h"

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
signals:
    //void results( const QList<QSqlRecord>& records );
    void results( bool success );
    void errorChange(GlobalError*);

private:
    void scanfolder();

    QSqlDatabase m_database;
    QDir backupFolder;
    QString createTablePref, bulkInsert, resultInsert;
    QStringList checkedFiles;
    QString queryStream;
};

#endif // WORKER_H
