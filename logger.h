#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>
#include <QFile>
#include "globalerror.h"
#include "circlequeue.h"


class Logger : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QStringList entries READ getEntries NOTIFY entriesChanged)
    Q_PROPERTY(bool visible READ getVisible WRITE setVisible NOTIFY visibleChanged)
private:
    //static Qml::Register::Controller<Logger> Register;
    QFile logfile;
    QTextStream logFileStream;
    //QStringList m_entries;
    CQueue<QString> m_entries;
    bool m_visible = false;
    bool fileCorrupted = true;
public:
    Logger();
    static Logger* instance();
    void setFile(const QString&);

    auto addEntry(GlobalError *) -> void;
    auto addEntryInFile(GlobalError *) -> void;
    auto getEntries() -> QStringList;
    auto setVisible(bool) -> void;
    auto getVisible() -> bool;
public slots:
    void clear();
signals:
    void entriesChanged();
    void visibleChanged();
    void errorChange(GlobalError*);
};

#endif // LOGGER_H
