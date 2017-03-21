#ifndef TRIPPLANNER_H
#define TRIPPLANNER_H

#include <QDialog>
#include <QTcpSocket>
#include <QStandardItemModel>

#include "fastretro.h"
#include "global.h"

class FastRetro : public QObject//, private Ui::TripPlanner
{
    Q_OBJECT
    Q_PROPERTY(bool startPermit READ startPermit WRITE setStartPermit NOTIFY startPermitChanged)
    Q_PROPERTY(bool stopPermit READ stopPermit WRITE setStopPermit NOTIFY stopPermitChanged)

public:
    FastRetro(QAbstractTableModel* model,QObject *parent = 0);

    enum {ConnectPermit = 0, ConnectEstablished};

    bool permitStart,permitStop;

    void setStartPermit(bool value){permitStart = value; emit startPermitChanged();}
    void setStopPermit(bool value){permitStop = value; emit stopPermitChanged();}

public slots:
    void dataChanged(QModelIndex,QModelIndex,QVector<int>);

    void connectToServer();
    void connectEstablished();
    void newDataAvailable();
    void stopScan();
    void connectionClosedByServer();
    void error();
    bool startPermit() {return permitStart;}
    bool stopPermit() {return permitStop;}

private:
    void closeConnection();
    void setCurState(int state);

    QAbstractTableModel* _model;

    QTcpSocket tcpSocket;
    quint16 nextBlockSize;
signals:
    void startPermitChanged();
    void stopPermitChanged();
};

#endif
