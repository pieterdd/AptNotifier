#include "calendardb.h"

#include "calendar.h"
#include <QFile>
#include <QTextStream>

CalendarDB::CalendarDB()
{
    _refreshInterval = 1;

    // Timer setup
    connect(&_autoUpdate, SIGNAL(timeout()), this, SLOT(updateCalendars()));
    _autoUpdate.setInterval(_refreshInterval*1000*60);
    _autoUpdate.start();
}

CalendarDB::~CalendarDB()
{
    _calLock.lock();
    for (QLinkedList<Calendar*>::iterator it = _calendars.begin();
         it != _calendars.end(); ++it) {
        delete *it;
    }
    _calLock.unlock();
}

void CalendarDB::loadCalendars()
{
    QFile file("calendars");

    if (!file.open(QIODevice::ReadOnly))
        return;

    // Read one URL per line
    while (!file.atEnd()) {
        // Vary calendar colors by varying hue values
        QColor calColor = composeNextColor();

        QString line = file.readLine();
        line = line.trimmed();
        addCalendar(line, calColor, false);
    }
}

void CalendarDB::addCalendar(const QString &url, const QColor &color, bool writeChange)
{

    // Create new calendar, trigger its first update
    Calendar* newCalendar = new Calendar(url, color);
    _calLock.lock();
    _calendars.push_back(newCalendar);
    emit newCalendarAdded(newCalendar);
    newCalendar->update();
    _calLock.unlock();

    // Write the updated list to the savefile if needed
    if (writeChange)
        writeCalendars();
}

void CalendarDB::removeCalendar(Calendar *cal)
{

    // Search for the pointer in the linked list
    _calLock.lock();
    for (QLinkedList<Calendar*>::iterator it = _calendars.begin();
         it != _calendars.end(); ++it) {
        if (*it == cal) {
            // Erase the calendar from the list
            _calendars.erase(it);
            emit removingCalendar(cal);
            delete cal;
            _calLock.unlock();

            // Write changes
            writeCalendars();
            return;
        }
    }
    _calLock.unlock();

    // If this point is reached, the calendar couldn't be found. This
    // behavior is likely caused by a bug, so it's a good idea to
    // raise SIGABRT here.
    abort();
}

QColor CalendarDB::composeNextColor()
{
    // Produce a hue value based on the next calendar ID
    QColor calColor(0, 0, 0);
    calColor.setHsv((_calendars.size() * 50) % 360, 255, 255);

    return calColor;
}

void CalendarDB::writeCalendars()
{
    QFile file("calendars");

    // Open file output
    if (!file.open(QIODevice::WriteOnly))
        return;
    QTextStream out(&file);

    // Write the URLs
    _calLock.lock();
    foreach (Calendar* cal, _calendars)
        out << cal->url() << "\n";
    _calLock.unlock();

    // Done.
    file.close();
}

void CalendarDB::updateCalendars()
{
    _calLock.lock();
    _autoUpdate.stop();

    // Run the update command on all calendars
    foreach (Calendar* item, _calendars)
        item->update();

    _autoUpdate.start(_autoUpdate.interval());
    _calLock.unlock();
}
