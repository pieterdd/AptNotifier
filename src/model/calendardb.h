#ifndef CALENDARDB_H
#define CALENDARDB_H

#include <QObject>

#include "calendar.h"
#include <QColor>
#include <QString>
#include <QLinkedList>

/**
  * Manages a list of calendars.
  * \author Pieter De Decker
  */
class CalendarDB : public QObject
{
    Q_OBJECT
public:
    CalendarDB();
    ~CalendarDB();

    /** Attempts to load previously saved calendars. */
    void loadCalendars();

    /** Adds calendar at a certain URL with a chosen color tag. Saves settings to file by default. */
    void addCalendar(const QString &url, const QColor& color, bool writeChange = true);

    /** Removes a certain calendar from the list and saves settings to file. Has
      * O(n) time complexity under the current list-based implementation. */
    void removeCalendar(Calendar* cal);

    /** Composes the color for the next calendar by varying the hue. */
    QColor composeNextColor();
private:
    static const char* CLASSNAME;

    /** Writes the current list of calendars to disk. */
    void writeCalendars();

    /** Sets up a single-shot timer for the next calendar refresh. */
    void scheduleUpdate();

    /** The list of all calendars. */
    QLinkedList<Calendar*> _calendars;
    QMutex _calLock;

    /* Update triggers */
    QTimer _autoUpdate;
    QMutex _autoUpdateLock;

    /** Number of minutes to wait before refreshing all calendars */
    int _refreshInterval;
public slots:
    /** Updates all calendars. If a calendar file hasn't changed (as determined
      * by the checksum), its buffers will not be re-populated. */
    void updateCalendars();
signals:
    /** Informs observers of a successfully added calendar */
    void newCalendarAdded(Calendar*);

    /** Warns observers that a calendar will be deleted */
    void removingCalendar(Calendar*);

    /** Broadcast when an invalid format is detected on a calendar. */
    void invalidFormatDetected(Calendar*);
};

#endif // CALENDARDB_H
