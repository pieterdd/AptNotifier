#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <QDateTime>

class QString;

/**
  * Stores details of a calendar event.
  * \author Pieter De Decker
  */
class Appointment
{
public:
    Appointment(const QString& rawData);
    Appointment(const Appointment& other);

    bool isValid() const { return _start.isValid() && _end.isValid(); }

    const QDateTime& start() const { return _start; }
    const QDateTime& end() const { return _end; }
    const QString& summary() const { return _summary; }

    /** Returns true if the appointment starts at midnight and its
      * duration is a multiple of 1 day. */
    bool isDayWide() const;

    /** Generates a string representation of the start/end time, relative
      * to the current time. */
    QString timeString() const;
private:
    // Time string generation helpers
    QString timeString_DayWide() const;
    QString timeString_Regular() const;

    // Parsing helpers
    void parseStart(const QString& rawData);
    void parseEnd(const QString& rawData);
    void parseSummary(const QString& rawData);
    QDateTime parseDateTime(const QString& rawData);

    QDateTime _start;
    QDateTime _end;
    QString _summary;
};

#endif // APPOINTMENT_H
