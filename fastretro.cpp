#include <QtGui>
#include <QtNetwork>

#include "fastretro.h"

FastRetro::FastRetro(QAbstractTableModel* model,QObject *parent)
    :_model(model), QObject(parent)
{
    /*
    setupUi(this);

    searchButton = buttonBox->addButton(tr("&Search"),
                                        QDialogButtonBox::ActionRole);
    stopButton = buttonBox->addButton(tr("S&top"),
                                      QDialogButtonBox::ActionRole);
    stopButton->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Close)->setText(tr("&Quit"));
    */
    setCurState(ConnectPermit);

    if(_model == nullptr)
        qWarning() << "Некорректная модель данных";

    QDateTime dateTime = QDateTime::currentDateTime();
    //dateEdit->setDate(dateTime.date());
    //timeEdit->setTime(QTime(dateTime.time().hour(), 0));
    /*
    progressBar->hide();
    progressBar->setSizePolicy(QSizePolicy::Preferred,
                               QSizePolicy::Ignored);

    tableWidget->verticalHeader()->hide();
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    connect(searchButton, SIGNAL(clicked()),
            this, SLOT(connectToServer()));
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stopSearch()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    */
    connect(_model, SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)), this, SLOT(dataChanged(QModelIndex,QModelIndex,QVector<int>)));
    connect(&tcpSocket, SIGNAL(connected()), this, SLOT(connectEstablished()));
    connect(&tcpSocket, SIGNAL(disconnected()),
            this, SLOT(connectionClosedByServer()));
    connect(&tcpSocket, SIGNAL(readyRead()),
            this, SLOT(newDataAvailable()));
    connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(error()));

}

void FastRetro::dataChanged(QModelIndex first,QModelIndex sec,QVector<int> roles)
{
//    qDebug() << "dataChanged emitted top: " << first.row() << " " <<  first.column();
//    qDebug() << "                    bottom: " << sec.row() << " " <<  sec.column();
//    if(!roles.isEmpty())
//        qDebug() << "                    roles: " << roles.first() << " " <<  roles.last();
}

void FastRetro::connectToServer()
{

    //tcpSocket.connectToHost(QHostAddress::LocalHost, 6178);
    int curRow(0);
    while(!_model->data(_model->index(curRow,ipColumn),ipRole).toString().isEmpty()) {
        QString server = _model->data(_model->index(curRow,ipColumn),ipRole).toString();

        qDebug() << server.left(server.indexOf(":")) << " and port " << server.right(server.size()-server.indexOf(":")-1);
        tcpSocket.connectToHost(QHostAddress(server.left(server.indexOf(":"))), server.right(server.size()-server.indexOf(":")-1).toUInt());
        _model->setData(_model->index(curRow,statusColumn),QString("Подключение"),statusRole);
        curRow++;
    }


/*
    tableWidget->setRowCount(0);
    searchButton->setEnabled(false);
    stopButton->setEnabled(true);
    statusLabel->setText(tr("Connecting to server..."));
    progressBar->show();
*/
    nextBlockSize = 0;
}

void FastRetro::connectEstablished()
{
    //\! 1 сокет
    setCurState(ConnectEstablished);
    _model->setData(_model->index(0,statusColumn),"Подключено",statusRole);
    /*
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_3);
    out << quint16(0) << quint8('S') << fromComboBox->currentText()
        << toComboBox->currentText() << dateEdit->date()
        << timeEdit->time();

    if (departureRadioButton->isChecked()) {
        out << quint8('D');
    } else {
        out << quint8('A');
    }
    out.device()->seek(0);
    out << quint16(block.size() - sizeof(quint16));
    tcpSocket.write(block);

    statusLabel->setText(tr("Sending request..."));
    */
}

void FastRetro::newDataAvailable()
{
    QDataStream in(&tcpSocket);
    //in.setVersion(QDataStream::Qt_4_3);

    forever {
        //int row = tableWidget->rowCount();

        if (nextBlockSize == 0) {
            if (tcpSocket.bytesAvailable() < sizeof(quint16))
                break;
            in >> nextBlockSize;
        }
        /*
        if (nextBlockSize == 0xFFFF) {
            closeConnection();
            statusLabel->setText(tr("Found %1 trip(s)").arg(row));
            break;
        }
        */
        if (tcpSocket.bytesAvailable() < nextBlockSize)
            break;

        QTime Time;

        in >> Time;
        qDebug() << Time;
        /*
        QStringList fields;
        fields << date.toString(Qt::LocalDate)
               << departureTime.toString(tr("hh:mm"))
               << arrivalTime.toString(tr("hh:mm"))
               << tr("%1 hr %2 min").arg(duration / 60)
                                    .arg(duration % 60)
               << QString::number(changes)
               << trainType;

        for (int i = 0; i < fields.count(); ++i)
            tableWidget->setItem(row, i,
                                 new QTableWidgetItem(fields[i]));
        */
        nextBlockSize = 0;
    }

}

void FastRetro::stopScan()
{
    //\! 1 сокет
    _model->setData(_model->index(0,statusColumn),"Не активен",statusRole);
    //statusLabel->setText(tr("Search stopped"));
    closeConnection();
}

void FastRetro::connectionClosedByServer()
{

    if (nextBlockSize != 0xFFFF)
        //\! 1 сокет
        _model->setData(_model->index(0,statusColumn),"Соединение закрыто сервером",statusRole);
    closeConnection();

}

void FastRetro::error()
{
    //\! 1 сокет
    _model->setData(_model->index(0,statusColumn),"Ошибка:"+tcpSocket.errorString(),statusRole);
    /*
    statusLabel->setText(tcpSocket.errorString());
    */
    closeConnection();

}

void FastRetro::closeConnection()
{
    tcpSocket.close();
    //searchButton->setEnabled(true);
    //stopButton->setEnabled(false);
    //progressBar->hide();

}

void FastRetro::setCurState(int state)
{
    switch (state) {
    case ConnectPermit:
        setStartPermit(true);
        setStopPermit(false);
        break;
    case ConnectEstablished:
        setStartPermit(false);
        setStopPermit(true);
        break;
    default:
        break;
    }
}
