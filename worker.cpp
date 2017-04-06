#include "worker.h"
#include <QDebug>
#include <QSqlQuery>

Worker::Worker(QObject *parent):QObject(parent),m_database(QSqlDatabase::addDatabase("QODBC"))
{
    ;
}

void Worker::addDatabase(const QString &driver)
{
    ;//qDebug() << "Worker:: Add database driver: " << driver;
    //m_database = ;
}

void Worker::slotExecute(const QString &query)
{
    qDebug() << "Worker:: Send SQL query";
    QSqlQuery queryClass;

    //qDebug() << query;
    if(queryClass.exec(query))
        emit results(true);
    else
        emit results(false);
}
