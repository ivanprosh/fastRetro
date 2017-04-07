#include "workthread.h"
#include "global.h"
#include <QDebug>

void WorkThread::run()
{
  qDebug() << "WorkThread:: init Worker";

  GLOBAL::globalMutex.lock();

  //qDebug() << "Point0";

  // forward to the worker: a 'queued connection'!
  connect( this, SIGNAL( queue( const QString& ) ),
             m_worker, SLOT( executeQuery( const QString& ) ) );

  //qDebug() << "Point2";
  // forward a signal back out
  connect( m_worker, SIGNAL( results( bool ) ),
           this, SIGNAL( queryFinished( bool ) ) );
  connect( this, SIGNAL( backupFolderNameChanged( const QString& ) ),
           m_worker, SLOT( setBackupFolder( const QString& ) ) );
  connect(m_worker, SIGNAL(errorChange(GlobalError*)),
          this, SIGNAL(errorChange(GlobalError*)));

  //qDebug() << "Point3";

  //m_mutex->unlock();
  qDebug() << "WorkThread:: finish init Worker, exec()";


  GLOBAL::globalMutex.unlock();

  exec();  // start our own event loop
}

WorkThread::WorkThread(QObject *parent):QThread(parent)
{
    // Create worker object within the context of the new thread
    m_worker = new Worker();
    qDebug() << "Point1";
}

WorkThread::~WorkThread()
{
    delete m_worker;
}

void WorkThread::execute( const QString& query )
{
    qDebug() << "WorkThread:: Emit Send SQL query to Worker";
    emit queue(query); // queues to worker
}

void WorkThread::setConnection(const QString &connection)
{
    qDebug() << "WorkThread:: Init new connection to db: " << connection;
    if(this->getWorker()->getDB().isValid() && this->getWorker()->getDB().isOpen())
        this->getWorker()->getDB().close();
    if(!connection.isEmpty())
        m_worker->getDB().setDatabaseName(connection);
}
