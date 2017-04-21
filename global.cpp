//#include <QApplication>

#include "global.h"

namespace GLOBAL {

//функция вычисляет i-й член последовательности Фиббоначи для таймеров.
int Fib(int i) {
    int value = 0;
    if(i < 1) return 0;
    if(i == 1) return 1;
    return Fib(i-1) + Fib(i - 2);
}

/*
void warning(QWidget *parent, const QString &title,
             const QString &text, const QString &detailedText)
{
    QScopedPointer<QMessageBox> messageBox(new QMessageBox(parent));

    if (parent)
        messageBox->setWindowModality(Qt::WindowModal);
    messageBox->setWindowTitle(QString("%1 - %2")
                               .arg(QApplication::applicationName()).arg(title));
    messageBox->setText(text);
    if (!detailedText.isEmpty())
        messageBox->setInformativeText(detailedText);
    messageBox->setIcon(QMessageBox::Warning);
    messageBox->addButton(QMessageBox::Ok);
    messageBox->exec();
}
*/
}
