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
    // Check if a start date has been given
    int dtstartPos = rawData.indexOf("DTSTART:");
    QString trimmedData = rawData.mid(dtstartPos + 8);
    if (dtstartPos == -1)
        return;

    // Parse the date
    _start = parseDateTime(trimmedData);
}

void Appointment::parseEnd(const QString &rawData)
{
    // Check if an end date has been given
    int dtendPos = rawData.indexOf("DTEND:");
    QString trimmedData = rawData.mid(dtendPos + 6);
    if (dtendPos == -1)
        return;

    // Parse the date
    _end = parseDateTime(trimmedData);
}

void Appointment::parseSummary(const QString &rawData) {
    // Check if a summary has been given
    int beginPos = rawData.indexOf("SUMMARY:");
    if (beginPos == -1)
        return;

    // Parse the summary
    QString input = rawData.mid(beginPos+8);
    _summary = input.left(input.indexOf("\r\n"));
}

QDateTime Appointment::parseDateTime(const QString& rawData) {
    QDate date(rawData.mid(0, 4).toInt(), rawData.mid(4, 2).toInt(), rawData.mid(6, 2).toInt());
    QTime time(rawData.mid(9, 2).toInt(), rawData.mid(11, 2).toInt(), rawData.mid(13, 2).toInt());

    QDateTime utcDT(date, time);
    return utcDT.addSecs(60*60*Calendar::getTimeShift());
}
