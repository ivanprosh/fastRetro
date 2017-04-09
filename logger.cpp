#include "logger.h"

Q_GLOBAL_STATIC(Logger, logger)

const int LogViewEntrySize = 100;

Logger::Logger():m_entries(LogViewEntrySize)
{

}

Logger* Logger::instance()
{
    return logger();
}

void Logger::setFile(const QString &_logFileName)
{
    if(!_logFileName.isEmpty()){
        logfile.setFileName(_logFileName);
    }
}

void Logger::clear() {
    this->m_entries.clear();
    emit this->entriesChanged();
}

auto Logger::addEntry(GlobalError* entry) -> void {
    if(!fileCorrupted && logFileStream.device()){
        addEntryInFile(entry);
    }
    this->m_entries.enqueue(entry->ErrorCodes.at(entry->firstItem()) + ":" + entry->secondItem());

    QMetaObject::invokeMethod(this, "entriesChanged", Qt::QueuedConnection);
}

void Logger::addEntryInFile(GlobalError* entry)
{
    //logfile.open(QIODevice::Write | QIODevice::Text);
    if(!logfile.open(QIODevice::WriteOnly | QIODevice::Text)){
        QScopedPointer<GlobalError> lastErr(new GlobalError(GlobalError::Logger,
                                                            "Ошибка открытия лог файла"));
        fileCorrupted = true;
        emit errorChange(lastErr.data());
    } else {
        fileCorrupted = false;
        logFileStream.setDevice(&logfile);
        logFileStream.setFieldAlignment(QTextStream::AlignLeft);
        logFileStream.setFieldWidth(25);
        logFileStream << entry->getDateTime().toString("yyyy.MM.dd hh:mm:ss");
        logFileStream.setFieldWidth(15);
        if(entry->ErrorCodes.size() > entry->firstItem()) {
            logFileStream << entry->ErrorCodes.at(entry->firstItem());//metaEnum.valueToKey(value->firstItem());
        } else {
            logFileStream << "UNKNOWN";
        }
        logFileStream.setFieldWidth(100);
        logFileStream << entry->secondItem();
        logFileStream << "\n";
        logfile.close();
    }
}

auto Logger::getEntries() -> QStringList {
    QStringList result;
    foreach (QString entry, this->m_entries) {
        result << entry;
    }
    return result;
}

auto Logger::setVisible(bool visible) -> void {
    if (this->m_visible != visible) {
        this->m_visible = visible;
        emit this->visibleChanged();
    }
}

auto Logger::getVisible() -> bool {
    return this->m_visible;
}
