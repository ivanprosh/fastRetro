#ifndef STRATEGIES_H
#define STRATEGIES_H

#include <QObject>
#include <QSqlDatabase>
#include <QDir>
#include <QFileSystemWatcher>
#include "globalerror.h"

//классы стратегий поведения
class AbstractStrategy: public QObject
{
   Q_OBJECT

protected:
   AbstractStrategy(QObject* parent = 0):QObject(parent),curError(new GlobalError(GlobalError::None,QString())){}
   virtual bool componentReady() {return true;}
   virtual void scanFolder() = 0;
   virtual void reinit() {}
   QString HistServerName;
   QDir backupFolder,
        HistServerFastFolder;
   QSharedPointer<GlobalError> curError;
public slots:
   void setHistorianPath(const QString& path);
   virtual void setBackupPath(const QString& path){
       backupFolder.setPath(path);
   }
signals:
   void errorChange(GlobalError*);
};

class Native : public AbstractStrategy
{
   Q_OBJECT

public:
    Native( QObject* parent = 0);
public slots:
    void backupDirectoryChanged(QString);
    void setBackupPath(const QString& path);
protected:
    void scanFolder() override;
    void reinit() override;
    bool componentReady() override { return !HistServerFastFolder.path().isEmpty();}
private:
    QFileSystemWatcher watcher;
};

class Forward : public AbstractStrategy
{
   Q_OBJECT

public:
    Forward( QObject* parent = 0);
    //~Forward(){}
    QSqlDatabase& getDB() {return m_database;}

protected:
    void scanFolder() override;
    bool componentReady() override {return (m_database.isValid() && m_database.isOpen());}
    void reinit() override;

public slots:
    bool executeQuery( const QString& query );
    void resultInsertQuery();

private:
    QSqlDatabase m_database;
};

#endif // STRATEGIES_H
