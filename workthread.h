#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QThread>
//#include "worker.h"
#include "global.h"
#include "globalerror.h"
#include "strategies.h"

//template<class Strategy>
class WorkThread: public QThread
{
    Q_OBJECT

  public:
    //WorkThread(QMutex* mutex, QObject *parent = Q_NULLPTR):QThread(parent),m_mutex(mutex){}
    WorkThread(QObject *parent = Q_NULLPTR):QThread(parent) {}
    ~WorkThread() {
        delete m_worker;
    }
    void execute( const QString& query )
    {
        emit queue(query); // queues to worker
    }

  protected:
    void run();

  signals:
    void queue( const QString& query );
    void queryFinished( bool success );
    void backupFolderNameChanged(const QString&);
    void errorChange(GlobalError*);
    void historianPathChanged(const QString&);
    //void initFinished();
  private:
#ifndef NATIVE
    Forward* m_worker;
#else
    Native* m_worker;
#endif

    //QMutex* m_mutex;
};


#endif // WORKTHREAD_H
