#include "icsparser.h"

#include "aptcache.h"
#include <cassert>
#include <QRegExp>

ICSParser::ICSParser(const QString& rawICS) {
    // Store the data
    _rawData = rawICS;
    _rawData.replace("\r", "");

    // Calculate the calendar checksum based on Last Modified attributes
    QString checksumData = _rawData;
    checksumData.replace(QRegExp("((^|\\n)(?!LAST-MODIFIED)[^\\n]*)+"), "");
    _checksum = qChecksum(checksumData.toUtf8(), checksumData.length());

    // If we can't checksum because Last Modified attributes are unavailable,
    // fall back on regular file checksumming.
    if (_checksum == 0)
        _checksum = qChecksum(_rawData.toUtf8(), _rawData.length());

    // Check if we can extract the calendar name
    int calNamePos = _rawData.indexOf("X-WR-CALNAME:");
    if (calNamePos != -1) {
        _name = _rawData.mid(calNamePos + 13);
        _name = _name.left(_name.indexOf("\n"));
    }
}

bool ICSParser::holdsValidICS() const {
    // If we don't have the ICS header, this is not a valid calendar
    if (_rawData.indexOf("BEGIN:VCALENDAR") != 0) {
        return false;
    }

    return true;
}

AptCache* ICSParser::readAppointments() const {
    QDateTime now = QDateTime::currentDateTime();
    QString rawData = _rawData;
    AptCache* aptCache = new AptCache();

    // Loop until all events are cached
    while (eventsLeft(rawData)) {
        QString calInfo = getRawApt(rawData);
        Appointment newApt(calInfo);

        // Add the newly extracted appointment if it hasn't already ended
        if (newApt.isValid() && now < newApt.end()) {
            aptCache->appointments()->insert(newApt.start(), newApt);

            // Create reminders where needed
            int triggerPos = calInfo.indexOf("TRIGGER:-P");
            while (triggerPos != -1) {
                QString triggerInfo = calInfo.mid(triggerPos + 10);
                triggerInfo = triggerInfo.left(triggerInfo.indexOf("\n"));

                // Only add reminder times that haven't passed yet
                QDateTime reminderStamp = constructReminderTime(newApt.start(), triggerInfo);
                assert(reminderStamp.isValid());
                if (now <= reminderStamp)
                    aptCache->reminders()->insert(reminderStamp, newApt);

                calInfo = calInfo.mid(triggerPos + 10);
                triggerPos = calInfo.indexOf("TRIGGER:-P");
            }
        }

        // Prepare for next appointment
        rawData = rawData.mid(rawData.indexOf("END:VEVENT") + 10);
    }

    return aptCache;
}

QDateTime ICSParser::constructReminderTime(const QDateTime& aptStart, const QString& triggerInfo) const {
    QDateTime reminderStamp = aptStart;
    QString triggerTmp = triggerInfo;

    // Extract offsets from the string and apply them to the timestamp
    int dayPos = triggerTmp.indexOf("D");
    if (dayPos != -1) {
        reminderStamp = reminderStamp.addDays(-triggerTmp.left(dayPos).toInt());
        triggerTmp = triggerTmp.mid(triggerTmp.indexOf(QRegExp("[0-9]"), dayPos));
    }

    int hourPos = triggerTmp.indexOf("H");
    if (hourPos != -1) {
        reminderStamp = reminderStamp.addSecs(-3600*triggerTmp.mid(0, hourPos).toInt());
        triggerTmp = triggerTmp.mid(triggerTmp.indexOf(QRegExp("[0-9]"), hourPos));
    }

    int minPos = triggerTmp.indexOf("M");
    if (minPos != -1) {
        reminderStamp = reminderStamp.addSecs(-60*triggerTmp.mid(0, minPos).toInt());
    }

    return reminderStamp;
}

bool ICSParser::eventsLeft(const QString& rawData) const {
    int beginPos = rawData.indexOf("BEGIN:VEVENT");
    int endPos = rawData.indexOf("END:VEVENT");

    if (beginPos == -1 || endPos == -1 || beginPos > endPos)
        return false;
    else
        return true;
}

QString ICSParser::getRawApt(const QString& rawData) const {
    int beginPos = rawData.indexOf("BEGIN:VEVENT");
    int endPos = rawData.indexOf("END:VEVENT");
    QString calInfo = rawData.mid(beginPos, endPos - beginPos);

    return calInfo;
}

QString ICSParser::trimCurrentApt(QString& rawData) const {
    int endPos = rawData.indexOf("END:VEVENT");
    assert(endPos != -1);
    rawData = rawData.mid(endPos + 10);
    return rawData;
}

