#include "workthread.h"

void WorkThread::run()
{
    qDebug() << "WorkThread:: init Worker";

    GLOBAL::globalMutex.lock();

#ifndef NATIVE
    m_worker = new Forward();
#else
    m_worker = new Native();
#endif
    // forward to the worker: a 'queued connection'!
    //connect( this, SIGNAL( queue( const QString& ) ),
    //           m_worker, SLOT( executeQuery( const QString& ) ) );

    //qDebug() << "Point2";
    // forward a signal back out
    connect( this, SIGNAL( historianPathChanged( const QString& ) ),
             m_worker, SLOT( setHistorianPath( const QString& ) ) );
    connect( this, SIGNAL( backupFolderNameChanged( const QString& ) ),
             m_worker, SLOT( setBackupPath( const QString& ) ) );
    //connect( m_worker, SIGNAL( results( bool ) ),
    //         this, SIGNAL( queryFinished( bool ) ) );
    connect(m_worker, SIGNAL(errorChange(GlobalError*)),
            this, SIGNAL(errorChange(GlobalError*)));

    //qDebug() << "Point3";


    qDebug() << "WorkThread:: finish init Worker, exec()";


    GLOBAL::globalMutex.unlock();

    exec();  // start our own event loop
}
