#include "workthread.h"
#include "global.h"
#include <QDebug>

void WorkThread::run()
{
  qDebug() << "WorkThread:: init Worker";

  GLOBAL::globalMutex.lock();

  qDebug() << "Point0";

  // Create worker object within the context of the new thread
  m_worker = new Worker();
  qDebug() << "Point1";

  // forward to the worker: a 'queued connection'!
  connect( this, SIGNAL( queue( const QString& ) ),
             m_worker, SLOT( slotExecute( const QString& ) ) );

  qDebug() << "Point2";
  // forward a signal back out
  connect( m_worker, SIGNAL( results( bool ) ),
           this, SIGNAL( queryFinished( bool ) ) );

  qDebug() << "Point3";

  //m_mutex->unlock();
  qDebug() << "WorkThread:: finish init Worker, exec()";


  GLOBAL::globalMutex.unlock();

  exec();  // start our own event loop
}

void WorkThread::execute( const QString& query )
{
    qDebug() << "WorkThread:: Emit Send SQL query to Worker";
    emit queue(query); // queues to worker
}

void WorkThread::setConnection(const QString &connection)
{
    qDebug() << "WorkThread:: Init new connection to db: " << connection;
    if(!connection.isEmpty())
        m_worker->getDB().setDatabaseName(connection);
}
