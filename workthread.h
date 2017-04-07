#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QThread>
#include "worker.h"

class WorkThread: public QThread
{
    Q_OBJECT

  public:
    //WorkThread(QMutex* mutex, QObject *parent = Q_NULLPTR):QThread(parent),m_mutex(mutex){}
    WorkThread(QObject *parent = Q_NULLPTR);
    ~WorkThread();

    void execute( const QString& query );
    void setConnection(const QString& serverName);
    Worker* getWorker(){return m_worker;}
  protected:
    void run();
  //public slots:
    //void slotExecQuery( const QString& query);
  signals:
    void queue( const QString& query );
    void queryFinished( bool success );
    void backupFolderNameChanged(const QString&);
    void errorChange(GlobalError*);
    //void initFinished();
  private:
    Worker* m_worker;
    //QMutex* m_mutex;
};

#endif // WORKTHREAD_H
