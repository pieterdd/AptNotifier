#include <QtGui/QApplication>

#include "model/logger.h"
#include "model/calendar.h"
#include "model/calendardb.h"
#include "view/calendardbview.h"
#include <QFile>
#include <QMutex>
#include <iostream>
#include <QMetaType>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
#ifndef DEBUG
    QCoreApplication::setApplicationName("AptNotifier");
#else
    QCoreApplication::setApplicationName("AptNotifier DEBUG");
#endif
    QCoreApplication::setApplicationVersion("-- Test version");

    CalendarDB calDB;
    CalendarDBView calDBView(&calDB);
    calDBView.hide();
    
    return a.exec();
}
