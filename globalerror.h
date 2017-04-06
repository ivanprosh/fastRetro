#ifndef GLOBALERROR_H
#define GLOBALERROR_H

#include <QObject>
#include <QDebug>

class GlobalError : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int idFrom READ idFrom WRITE setIdFrom NOTIFY idFromChanged)
    Q_PROPERTY(ErrorRoles firstItem READ firstItem NOTIFY firstItemChanged)
    Q_PROPERTY(QString secondItem READ secondItem NOTIFY secondItemChanged)
    Q_ENUMS(ErrorRoles)
public:
    enum ErrorRoles {Socket=1,Configuration,Historian,None=10};

    GlobalError(QObject *parent = 0):pair(),QObject(parent){}
    GlobalError(ErrorRoles role,const QString& val):pair(role,val){}

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
    void setIdFrom(int idfrom){_idfrom=idfrom; emit idFromChanged();}
    int idFrom(){return _idfrom;}
private:
    QPair<ErrorRoles, QString> pair;
    int _idfrom;
signals:
    void firstItemChanged();
    void secondItemChanged();
    void idFromChanged();
};


#endif // GLOBALERROR_H
