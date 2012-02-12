/**
 * \class Appointment
 * \author Pieter De Decker
 *
 * \brief Stores details of a calendar event.
 */
#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <QDateTime>

class QString;

class Appointment
{
public:
    Appointment(const QString& rawData);
    Appointment(const Appointment& other);

    bool isValid() const { return _valid; }

    const QDateTime& start() const { return _start; }
    const QDateTime& end() const { return _end; }
    const QString& summary() const { return _summary; }

    /** Converts a timestamp into 'hh:mm', 'Jan 1', 'tomorrow', etc. */
    static QString composeShortDateTime(const QDateTime& dateTime);
private:
    // Parsing helpers
    void parseStart(const QString& rawData);
    void parseEnd(const QString& rawData);
    void parseSummary(const QString& rawData);
    QDateTime parseDateTime(const QString& rawData);

    QDateTime _start;
    QDateTime _end;
    QString _summary;

    bool _valid;
};

#endif // APPOINTMENT_H
