#ifndef WORKER_H
#define WORKER_H

#include <QSqlDatabase>
#include <QDir>
#include <QFileSystemWatcher>
#include "globalerror.h"
#include "strategies.h"
/*
template<class StrategyPar>
class Worker : public StrategyPar
{
    Q_OBJECT

public slots:
    void setBackupFolder( const QString& );
    void historianPathChanged(const QString& );

signals:
    void results( bool success );
};

template<typename StrategyPar> void Worker<StrategyPar>::setBackupFolder(const QString &path)
{
    StrategyPar::setBackupPath(path);
}

template<typename StrategyPar> void Worker<StrategyPar>::historianPathChanged(const QString &path)
{
    qDebug() << "Worker:: Init new path to historian: " << path;
    StrategyPar::setHistorianPath(path);
}
*/
#endif // WORKER_H
