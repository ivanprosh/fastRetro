#include "addresstable.h"
#include "global.h"

#include <QDebug>
#include <QRegExp>

namespace {
const int MaxColumns = 2;
}

AddressTable::AddressTable(QObject *parent)
    : QAbstractTableModel(parent)
{
    initialize();
}

void AddressTable::initialize()
{
    hash = new QHash<int,QByteArray>;
    hash->insert(ipRole, "ipAddr");
    hash->insert(statusRole, "status");

    //setHorizontalHeaderLabels(QStringList() << tr("IP адрес") << tr("Статус"));

    //добавим строки
    items.push_back(QPair<QString,QString>("172.16.3.121:10000","Не активен"));

    int row(1);
    while(row < MAX_CONNECTIONS_COUNT){
        items.push_back(QPair<QString,QString>("",""));
        row++;
    }
}

void AddressTable::clear()
{
    //QStandardItemModel::clear();
    initialize();
}

void AddressTable::ipChange(const int curRow, const int curColumn, const QVariant &value)
{
//    qDebug() << "row " << curRow << "column " << curColumn << "role " <<  Qt::UserRole+curColumn;
//    qDebug() << value.toString();

    QRegExp correctIp("^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\:(\\d+)");

    if(index(curRow,curColumn).isValid() && (correctIp.exactMatch(value.toString()))){
        if(correctIp.cap(1).toInt() < 255 || correctIp.cap(2).toInt() < 255 || correctIp.cap(3).toInt() < 255 || correctIp.cap(4).toInt() < 255){
            //setData(index(curRow,curColumn), value, Qt::UserRole+curColumn);
            setData(index(curRow,curColumn), value, ipRole);
            setData(index(curRow,curColumn), QString("Не активен"), statusRole);
        }
    }
    else if(value.toString().isEmpty()){
        //setData(index(curRow,curColumn), value, Qt::UserRole+curColumn);
        setData(index(curRow,curColumn), value, ipRole);
        setData(index(curRow,curColumn), QString(), statusRole);
    }

}

bool AddressTable::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && (role == Qt::EditRole || role >= Qt::UserRole)) {
        qDebug() << role << " " << value.toString();

        Q_ASSERT(index.row()<=items.size());

        if(items.empty()){
            Q_ASSERT(role==statusRole);
            items.push_back(QPair<QString,QString>(value.toString(),"Не активен"));
        } else {
            switch (role) {
            case ipRole:
                if(items.at(index.row()).first == value.toString())
                    return false;
                items[index.row()].first = value.toString();
                break;
            case statusRole:
                if(items.at(index.row()).second == value.toString())
                    return false;
                items[index.row()].second = value.toString();
                break;
            default:
                return false;
            }
        }

        emit dataChanged(index, index);
        return true;
    }
    return false;
}

int AddressTable::rowCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : items.size();
}


int AddressTable::columnCount(const QModelIndex &index) const
{
    return index.isValid() ? 0 : MaxColumns;
}

QVariant AddressTable::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();

    //qDebug() << index.row() << items.size();
    Q_ASSERT(index.row()<=items.size());
    switch (role) {
    case ipRole:
        return items.at(index.row()).first;
    case statusRole:
        return items.at(index.row()).second;
    default:
        Q_ASSERT(false);
    }
}

QHash<int, QByteArray> AddressTable::roleNames() const
{
    return *hash;
}
Qt::ItemFlags AddressTable::flags(const QModelIndex &index) const
{
    Qt::ItemFlags theFlags = QAbstractTableModel::flags(index);
    if (index.isValid())
        theFlags |= Qt::ItemIsSelectable|Qt::ItemIsEditable|
                    Qt::ItemIsEnabled;
    return theFlags;
}
QVariant AddressTable::headerData(int section,
        Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case ipColumn: return tr("IP адрес");
            case statusColumn: return tr("Статус");
            default: Q_ASSERT(false);
        }
    }
    return section + 1;
}


