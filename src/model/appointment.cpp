#include "appointment.h"

#include "calendar.h"
#include <cmath>
#include <cassert>
#include <QString>

Appointment::Appointment(const QString& input)
{
    QString rawData = input;

    parseStart(rawData);
    parseEnd(rawData);
    parseSummary(rawData);
}

Appointment::Appointment(const Appointment& other) {
    _start = other._start;
    _end = other._end;
    _summary = other._summary;
}

bool Appointment::isDayWide() const {
    assert(isValid());

    // Rule 1: the appointment must start exactly at midnight
    if (_start.time().hour() != 0 || _start.time().minute() != 0 || _start.time().second() != 0)
        return false;

    // Rule 2: the appointment must end exactly at midnight
    if (_end.time().hour() != 0 || _end.time().minute() != 0 || _end.time().second() != 0)
        return false;

    return true;
}

QString Appointment::timeString() const {
    // Day-wide appointment strings are formatted
    // differently than their regular counterparts.
    if (isDayWide())
        return timeString_DayWide();
    else
        return timeString_Regular();
}

QString Appointment::timeString_DayWide() const {
    QDateTime now = QDateTime::currentDateTime();

    // Rule 1: for a day-wide appointment with a 1-day duration...
    if (_start.daysTo(_end) == 1) {
        // ... if it starts today
        if (now.date() == _start.date())
            return "Today";
        // ... if it starts tomorrow
        else if (now.date().addDays(1) == _start.date())
            return "Tomorrow";
        // ... if it starts later
        else
            return _start.toString(Qt::SystemLocaleLongDate);
    }
    // Rule 2: for a day-wide appointment lasting longer than 1 day...
    else {
        // ... with a passed start date
        if (now.date() > _start.date())
            return "Started " + _start.date().toString(Qt::SystemLocaleShortDate) + " (" + QString::number(now.daysTo(_end)) + " days left)";
        // ... if it starts today
        else if (now.date() == _start.date())
            return "Started today (" + QString::number(now.daysTo(_end)) + " days left)";
        // ... if it starts tomorrow
        else if (now.date().addDays(1) == _start.date())
            return "Starts tomorrow (lasts " + QString::number(_start.daysTo(_end)) + " days)";
        // ... with a start date in the future
        else
            return "Starts " + _start.date().toString(Qt::SystemLocaleShortDate) + " (in " + QString::number(now.daysTo(_start)) + " days)";
    }
}

QString Appointment::timeString_Regular() const {
    QDateTime now = QDateTime::currentDateTime();

    // Rule 1: the appointment starts today...
    if (now.date() == _start.date()) {
        // ... and started this very minute
        if (_start.time() == now.time().addSecs(-now.time().second()))
            return "Just started";
        // ... and its start time has passed
        else if (_start < now)
            return QString::number(ceil(now.secsTo(_end)/60.0)) + " minutes left";
        // ... and starts within the next hour
        else if (_start < now.addSecs(60*60))
            return "In " + QString::number(ceil(now.secsTo(_start)/60.0)) + " minutes";
        // ... and starts a later time today
        else
            return "In " + QString::number(ceil(now.secsTo(_start)/60.0/60.0)) + " hours";
    }
    // Rule 2: other cases
    else
        return "Starts " + _start.date().toString(Qt::SystemLocaleShortDate) + " " + _start.time().toString("hh:mm");
}

void Appointment::parseStart(const QString &rawData)
{
    // Option 1: the start time is a date/time stamp
    int dtstartPos = rawData.indexOf("DTSTART:");
    if (dtstartPos != -1) {
        QString trimmedData = rawData.mid(dtstartPos + 8);
        trimmedData = trimmedData.left(trimmedData.indexOf("\n"));
        _start = parseDateTime(trimmedData);
    }

    // Option 2: the start time is a date stamp
    dtstartPos = rawData.indexOf("DTSTART;VALUE=DATE:");
    if (dtstartPos != -1) {
        QString trimmedData = rawData.mid(dtstartPos + 19);
        trimmedData = trimmedData.left(trimmedData.indexOf("\n"));
        QDate date(trimmedData.mid(0, 4).toInt(), trimmedData.mid(4, 2).toInt(), trimmedData.mid(6, 2).toInt());
        _start = QDateTime(date);
    }
}

void Appointment::parseEnd(const QString &rawData)
{
    // Option 1: the end time is a date/time stamp
    int dtendPos = rawData.indexOf("DTEND:");
    if (dtendPos != -1) {
        QString trimmedData = rawData.mid(dtendPos + 6);
        _end = parseDateTime(trimmedData);
    }

    // Option 2: the end time is a date stamp
    dtendPos = rawData.indexOf("DTEND;VALUE=DATE:");
    if (dtendPos != -1) {
        QString trimmedData = rawData.mid(dtendPos + 17);
        trimmedData = trimmedData.left(trimmedData.indexOf("\n"));
        QDate date(trimmedData.mid(0, 4).toInt(), trimmedData.mid(4, 2).toInt(), trimmedData.mid(6, 2).toInt());
        _end = QDateTime(date);
    }

}

void Appointment::parseSummary(const QString &rawData) {
    // Check if a summary has been given
    int beginPos = rawData.indexOf("SUMMARY:");
    if (beginPos == -1)
        return;

    // Parse the summary, taking escape characters into account
    QString input = rawData.mid(beginPos+8);
    _summary = input.left(input.indexOf("\n"));
    _summary = _summary.replace("\\,", ",");
    _summary = _summary.replace("\\n", "\n");
    _summary = _summary.replace("\\;", ";");
    _summary = _summary.replace("\\\\", "\\");
}

QDateTime Appointment::parseDateTime(const QString& rawData) {
    QDate date(rawData.mid(0, 4).toInt(), rawData.mid(4, 2).toInt(), rawData.mid(6, 2).toInt());
    QTime time(rawData.mid(9, 2).toInt(), rawData.mid(11, 2).toInt(), rawData.mid(13, 2).toInt());

    QDateTime utcDT(date, time);
    return utcDT.addSecs(60*60*Calendar::getTimeShift());
}
