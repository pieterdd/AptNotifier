#include "calendardb.h"

#include "calendar.h"
#include <QFile>
#include <QTextStream>

CalendarDB::CalendarDB()
{
}

CalendarDB::~CalendarDB()
{
    for (QLinkedList<Calendar*>::iterator it = _calendars.begin();
         it != _calendars.end(); ++it) {
        delete *it;
    }
}

void CalendarDB::loadCalendars()
{
    QFile file("calendars");

    if (!file.open(QIODevice::ReadOnly))
        return;

    // Read one URL per line
    while (!file.atEnd()) {
        QString line = file.readLine();
        line = line.trimmed();
        addCalendar(line, QColor(255, 0, 0), false); // TODO: vary colors
    }
}

void CalendarDB::addCalendar(const QString &url, const QColor &color, bool writeChange)
{
    _calendars.push_back(new Calendar(url, color));
    emit newCalendarAdded(_calendars.last());

    // Write the updated list to the savefile if needed
    if (writeChange)
        writeCalendars();
}

void CalendarDB::removeCalendar(Calendar *cal)
{
    // Search for the pointer in the linked list
    for (QLinkedList<Calendar*>::iterator it = _calendars.begin();
         it != _calendars.end(); ++it) {
        if (*it == cal) {
            _calendars.erase(it);
            emit removingCalendar(cal);
            delete cal;
            return;
        }
    }

    // If this point is reached, the calendar couldn't be found. This
    // behavior is likely caused by a bug, so it's a good idea to
    // raise SIGABRT here.
    abort();
}

void CalendarDB::writeCalendars()
{
    QFile file("calendars");

    if (!file.open(QIODevice::WriteOnly))
        return;

    QTextStream out(&file);
    foreach (Calendar* cal, _calendars)
        out << cal->url() << "\n";

    file.close();
}
