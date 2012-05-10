#include <QtGui/QApplication>

#include "model/calendar.h"
#include "model/calendardb.h"
#include "view/calendardbview.h"
#include <QMetaType>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CalendarDB calDB;
    CalendarDBView calDBView(&calDB);
    calDBView.hide();
    
    return a.exec();
}

//#include "view/toaster/toaster.h"
//#include "view/toaster/appointmentlist.h"

//int main(int argc, char* argv[]) {
//    QApplication a(argc, argv);

//    Toaster tst("Ongoing Event", new AppointmentList(NULL, QLinkedList<Appointment>()));
//    tst.show();

//    return a.exec();
//}
