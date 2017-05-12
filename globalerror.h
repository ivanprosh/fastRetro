#ifndef GLOBALERROR_H
#define GLOBALERROR_H

#include <QObject>
#include <QDebug>
#include <QDateTime>

class GlobalError : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int plcIdFrom READ plcIdFrom WRITE setPlcIdFrom NOTIFY plcIdFromChanged)
    Q_PROPERTY(QString from READ from WRITE setFrom NOTIFY fromChanged)
    Q_PROPERTY(ErrorRoles firstItem READ firstItem NOTIFY firstItemChanged)
    Q_PROPERTY(QString secondItem READ secondItem NOTIFY secondItemChanged)
    Q_ENUMS(ErrorRoles)

public:
    enum ErrorRoles {Socket=1,Configuration,Historian,System,Logger,Debug,None=10};

    QStringList ErrorCodes;//QStringList() << "Socket" << "Configuration" << "Historian" << "System"
    //GlobalError(QObject *parent = 0):pair(),QObject(parent){}
    GlobalError(ErrorRoles role = None,const QString& val = QString("Ok"), QObject *parent = 0):
        QObject(parent),pair(role,val),_from(""),_plcIdFrom(-1){
        eventDateTime = QDateTime::currentDateTime();
        ErrorCodes << "" << "сокет" << "конфигурация" << "архиватор" << "общие" << "лог" << "отладка";
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

    void setFrom(QString from){_from=from; emit fromChanged();}
    void setPlcIdFrom(int id){_plcIdFrom=id; emit plcIdFromChanged();}

    QString from(){return _from;}
    int plcIdFrom() {return _plcIdFrom;}
private:
    QPair<ErrorRoles, QString> pair;
    QDateTime eventDateTime;
    QString _from;
    int _plcIdFrom;
signals:
    void firstItemChanged();
    void secondItemChanged();
    void fromChanged();
    void plcIdFromChanged();
};


#endif // GLOBALERROR_H
