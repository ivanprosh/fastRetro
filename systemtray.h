#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QObject>
#include <QAction>
#include <QSystemTrayIcon>

class SystemTray : public QObject
{
    Q_OBJECT

public:
    explicit SystemTray(QObject *parent = 0);

    // Сигналы от системного трея
signals:
    void signalIconActivated();
    void signalShow();
    void signalQuit();

private slots:
    /* Слот, который будет принимать сигнал от события
     * нажатия на иконку приложения в трее
     */
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

public slots:
    void hideIconTray();

private:
    /* Объявляем объект будущей иконки приложения для трея */
    QSystemTrayIcon         * trayIcon;
};

#endif // SYSTEMTRAY_H
