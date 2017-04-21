#ifndef GLOBAL_H
#define GLOBAL_H

#include <QMutex>
#include <QObject>

const int MAX_PAR_COUNT = 20;
const int MAX_CONNECTIONS_COUNT = 10;

enum Roles{ipRole = Qt::UserRole, statusRole};
enum Columns{ipColumn = 0, statusColumn};

namespace GLOBAL {

    extern QMutex globalMutex;
//enum ErrorRoles;

//    void warning(QWidget *parent, const QString &title,
//             const QString &text, const QString &detailedText);
   int Fib(int i);
}


#endif // GLOBAL_H
