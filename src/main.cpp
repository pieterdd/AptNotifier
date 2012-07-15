#include <QtGui/QApplication>

#include "stringres.h"
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
    a.setApplicationName("AptNotifier");
#else
    a.setApplicationName("AptNotifier DEBUG");
#endif
    a.setApplicationVersion("-- Test version");

    CalendarDB calDB;
    CalendarDBView calDBView(&calDB);
    calDBView.hide();
    
    return a.exec();
}
