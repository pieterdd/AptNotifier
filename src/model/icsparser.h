#ifndef ICSPARSER_H
#define ICSPARSER_H

#include <QString>
#include <QDateTime>

class AptCache;

/**
  * Parses an ICS calendar file, which is supplied as a string in
  * the object constructor.
  * \author Pieter De Decker
  */
class ICSParser
{
public:
    ICSParser(const QString& rawICS);

    /** Does an elementary validity check on an alleged ICS file. */
    bool holdsValidICS() const;

    /** Allocates an AptCache structure filled with all events found
      * in the ICS file. Ownership of the AptCache structure is
      * transferred to the caller. */
    AptCache* readAppointments() const;

    /** Getter for the calendar checksum that is used to detect
      * changes that occured between two calendar downloads. */
    int checksum() const { return _checksum; }

    /** Getter for the calendar name, as stored by the X-WR-CALNAME
      * property. */
    QString name() const { return _name; }

private:
    /** Determines the reminder timestamp of an appointment based on the appointment
      * time and the "TRIGGER" field. */
    QDateTime constructReminderTime(const QDateTime& aptStart, const QString& triggerInfo) const;

    /** Gets the raw data inside the next appointment. */
    QString getRawApt(const QString& rawData) const;

    /** Trims the frontmost appointment from _rawData. */
    QString trimCurrentApt(QString& rawData) const;

    /** If there are any events left in the ICS file, this function
      * returns true. */
    bool eventsLeft(const QString& rawData) const;

    QString _rawData;
    int _checksum;
    QString _name;
};

#endif // ICSPARSER_H
