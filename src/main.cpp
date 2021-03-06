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
    QCoreApplication::setApplicationVersion("v0.01");
    Logger::instance()->initialize();

    CalendarDB calDB;
    CalendarDBView calDBView(&calDB);
    calDBView.hide();
    
    return a.exec();
}
