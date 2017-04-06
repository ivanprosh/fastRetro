#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QSqlDatabase>

class Worker : public QObject
{
    Q_OBJECT

public:
    Worker( QObject* parent = 0);
    ~Worker(){}
    QSqlDatabase& getDB() {return m_database;}
    void addDatabase(const QString& driver="QODBC");

public slots:
    void slotExecute( const QString& query );

signals:
    //void results( const QList<QSqlRecord>& records );
    void results( bool success );

private:
    QSqlDatabase m_database;
};

#endif // WORKER_H
