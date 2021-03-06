#ifndef GLOBALERROR_H
#define GLOBALERROR_H

#include <QObject>
#include <QDebug>
#include <QDateTime>

class GlobalError : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int idFrom READ idFrom WRITE setIdFrom NOTIFY idFromChanged)
    Q_PROPERTY(ErrorRoles firstItem READ firstItem NOTIFY firstItemChanged)
    Q_PROPERTY(QString secondItem READ secondItem NOTIFY secondItemChanged)
    Q_ENUMS(ErrorRoles)

public:
    enum ErrorRoles {Socket=1,Configuration,Historian,System,Logger,None=10};

    QStringList ErrorCodes;//QStringList() << "Socket" << "Configuration" << "Historian" << "System"
    GlobalError(QObject *parent = 0):pair(),QObject(parent){}
    GlobalError(ErrorRoles role,const QString& val):pair(role,val){
        eventDateTime = QDateTime::currentDateTime();
        ErrorCodes << "" << "сокет" << "конфигурация" << "архиватор" << "общие" << "лог";
    }

    ErrorRoles firstItem() {return pair.first;}
    QString secondItem() {return pair.second;}

    void setFirstItem(ErrorRoles role){
        pair.first = role;
        emit firstItemChanged();
    }
    void setSecondItem(QString value){
        pair.second = value;
        qDebug() << "GlobalError::Second item changed";
        emit secondItemChanged();
    }
    QDateTime getDateTime(){return eventDateTime;}
    void setIdFrom(int idfrom){_idfrom=idfrom; emit idFromChanged();}
    int idFrom(){return _idfrom;}
private:
    QPair<ErrorRoles, QString> pair;
    QDateTime eventDateTime;
    int _idfrom;
signals:
    void firstItemChanged();
    void secondItemChanged();
    void idFromChanged();
};


#endif // GLOBALERROR_H
