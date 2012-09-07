#include "appointment.h"

#include "calendar.h"
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

QString Appointment::composeShortDateTime(const QDateTime &dateTime)
{
    QDateTime now = QDateTime::currentDateTime();

    // Rule 1: the timestamp falls within this day, show the time code
    if (now.date() == dateTime.date())
        return QString::number(dateTime.time().hour()) + ":" + QString().sprintf("%02d", dateTime.time().minute());
    // Rule 2: the timestamp falls within this year, show the month and day
    else if (now.date().year() == dateTime.date().year())
        return QDate::shortMonthName(dateTime.date().month()) + " " + dateTime.date().day();
    // Rule 3: the timestamp doesn't fall within this year, show short date format
    else
        return dateTime.date().toString(Qt::SystemLocaleShortDate);
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
