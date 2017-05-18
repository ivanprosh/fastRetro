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
    //задание шапки с форматированием
    if(!_logFileName.isEmpty()){
        logfile.setFileName(_logFileName);
        fileCorrupted = false;
        lastFilePos = 0;
        if(logfile.open(QIODevice::ReadWrite | QIODevice::Text) && logfile.size()==0){
            logFileStream.setDevice(&logfile);
            logFileStream.setFieldAlignment(QTextStream::AlignCenter);
            logFileStream.setFieldWidth(20);
            logFileStream << QString("Время");
            logFileStream.setFieldWidth(10);
            logFileStream << QString("Тип");
            logFileStream.setFieldWidth(30);
            logFileStream << QString("Идентификатор");
            logFileStream.setFieldWidth(80);
            logFileStream << QString("Текст сообщения");
        }
        logfile.close();
    }
}

void Logger::clear() {
    this->m_entries.clear();
    emit this->entriesChanged();
}

auto Logger::addEntry(GlobalError* entry) -> void {
    //добавление в выходной файл
    addEntryInFile(entry);
    //добавление в очередь сообщений
    this->m_entries.enqueue(entry->getDateTime().time().toString("hh:mm:ss")
                            + " "
                            + entry->ErrorCodes.at(entry->firstItem())
                            + "\t"
                            + entry->from()
                            + "\t"
                            + entry->secondItem());

    QMetaObject::invokeMethod(this, "entriesChanged", Qt::QueuedConnection);
}

void Logger::addEntryInFile(GlobalError* entry)
{
    //logfile.open(QIODevice::Write | QIODevice::Text);
    if(!logfile.open(QIODevice::ReadWrite | QIODevice::Text)){
        QScopedPointer<GlobalError> lastErr(new GlobalError(GlobalError::Logger,
                                                            "Ошибка открытия лог файла"));
        fileCorrupted = true;
        emit errorChange(lastErr.data());
    } else {
        fileCorrupted = false;
        logFileStream.setDevice(&logfile);
        logFileStream.seek(logfile.size());
        logFileStream.setFieldAlignment(QTextStream::AlignLeft);
        //logFileStream.setFieldWidth(15);
        logFileStream << "\n" << entry->getDateTime().toString("yyyy.MM.dd hh:mm:ss\t");
        logFileStream.setFieldWidth(15);
        if(entry->ErrorCodes.size() > entry->firstItem()) {
            logFileStream << entry->ErrorCodes.at(entry->firstItem());//metaEnum.valueToKey(value->firstItem());
        } else {
            logFileStream << "UNKNOWN";
        }
        logFileStream.setFieldWidth(30);
        logFileStream << entry->from();
        //logFileStream.setFieldWidth(100);
        logFileStream << entry->secondItem();
        //logFileStream << "\n";
        lastFilePos = logFileStream.pos();

        logfile.close();
        qDebug() << "Logger:: Current pos of logfile" << logfile.size();
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
