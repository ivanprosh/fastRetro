#ifndef ADDRESSTABLE_H
#define ADDRESSTABLE_H

#include <QAbstractTableModel>

class AddressTable : public QAbstractTableModel
{
    Q_OBJECT

private:
    QVector<QPair<QString,QString> > items;
public:

    AddressTable(QObject *parent = 0);

    QVariant data(const QModelIndex &index,
                  int role=Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role=Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent=QModelIndex()) const;
    int columnCount(const QModelIndex &parent=QModelIndex()) const;

    bool setData(const QModelIndex &item, const QVariant &value, int role);
    //QString userdata(int row,int role) const;
    //QString curRecName;
    void initialize();
    void clear();
    Q_INVOKABLE void ipChange(const int curRow,const int curColumn,const QVariant &value);
    //bool setData(const QModelIndex &item, const QVariant &value, int role = Qt::EditRole);
protected:
    QHash<int,QByteArray> *hash;
    QHash<int,QByteArray> roleNames() const;
public slots:
    //set

};

#endif // ADDRESSTABLE_H
