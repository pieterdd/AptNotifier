#include <QtGui/QApplication>

#include "model/calendar.h"
#include "model/calendardb.h"
#include "view/calendardbview.h"
#include <QMetaType>

int main(int argc, char *argv[])
{
    qRegisterMetaType<Calendar::ExceptionType>("ExceptionType");
    QApplication a(argc, argv);

    CalendarDB calDB;
    CalendarDBView calDBView(&calDB);
    calDBView.hide();
    
    int returnCode = a.exec();
    return returnCode;
}
