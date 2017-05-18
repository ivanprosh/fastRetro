#include "workthread.h"

void WorkThread::run()
{
    qDebug() << "WorkThread:: init Worker";

    GLOBAL::globalMutex.lock();
/*
 * класс worker создан для того, чтобы сигнально-слотовые события выполнялись
 * в отдельном потоке, а не в основном, так как слоты WorkThread будут вызываться из
 * основного треда, то транслируем их в отдельный тред. Особенно актуально, если будет
 * использоваться Forward стратегия. Например, подключение к базе при смене имени сервера не будет вешать интерфейс пользователя
 */
#ifndef NATIVE
    m_worker = new Forward();
#else
    m_worker = new Native();
#endif

    connect( this, SIGNAL( historianPathChanged( const QString& ) ),
             m_worker, SLOT( setHistorianPath( const QString& ) ) );
    connect( this, SIGNAL( backupFolderNameChanged( const QString& ) ),
             m_worker, SLOT( setBackupPath( const QString& ) ) );
    connect(m_worker, SIGNAL(errorChange(GlobalError*)),
            this, SIGNAL(errorChange(GlobalError*)));


    qDebug() << "WorkThread:: finish init Worker, exec()";


    GLOBAL::globalMutex.unlock();
    //собственная петля для сигнально-слотовой системы
    exec();  // start our own event loop
}
