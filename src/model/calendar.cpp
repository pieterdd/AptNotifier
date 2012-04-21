#include "calendar.h"

#include "appointment.h"
#include <cmath>
#include <QFile>
#include <QDebug>
#include <cassert>
#include <QTextStream>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>

short Calendar::timeShift = Calendar::calcTimeShift();
const short Calendar::IMAGEDIM = 64;

Calendar::Calendar(const QString &url, const QColor &color)
    : _naMgr(this)
{
    QByteArray qbaUrl;
    _url = QUrl::fromEncoded(qbaUrl.append(url));
    _name = "Untitled Calendar";
    _calChecksum = 0;
    _color = color;
    _status = NotLoaded;
    buildCalendarImage();

    // Wire QObjects
    connect(&_nfyTimer, SIGNAL(timeout()), this, SLOT(sendNotifications()));
    connect(&_naMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(parseNetworkResponse(QNetworkReply*)));
}

Calendar::~Calendar() {
}

void Calendar::update()
{
    // Start calendar download asynchronously. After retrieving the
    // file, parseNetworkResponse will take over.
    _naMgr.get(QNetworkRequest(_url));
}

void Calendar::sendNotifications()
{
    _bufferLock.lock();

    // First get the ongoing appointment notifications out the door,
    // then process the reminders.
    sendNotifications_Ongoing();
    sendNotifications_Reminders();

    // Check notifications again in half a minute
    QTimer::singleShot(30000, this, SLOT(sendNotifications()));

    _bufferLock.unlock();
}

void Calendar::sendNotifications_Ongoing()
{
    QDateTime now = QDateTime::currentDateTime();
    now = now.addSecs(-now.time().second());
    QLinkedList<Appointment> newOngoing;

    // Remove appointments that are no longer ongoing
    for (QLinkedList<Appointment>::iterator it = _ongoingApts.begin();
         it != _ongoingApts.end(); ++it) {
        const Appointment& apt = *it;

        if (apt.end() < now)
            it = _ongoingApts.erase(it);
    }

    // Collect newly ongoing appointments and transfer them to a separate list
    bool foundAllNewlyOngoing = false;
    QMultiMap<QDateTime, Appointment>::iterator it = _appointments.begin();
    while (it != _appointments.end() && !foundAllNewlyOngoing) {
        const Appointment& apt = it.value();

        // Stop searching if the selected appointment hasn't started yet
        if (now < apt.start()) {
            foundAllNewlyOngoing = true;
        }
        // Transfer ongoing appointments to a separate data structure
        else if (apt.start() <= now && now <= apt.end()) {
            // Add it to the ongoing list and the "new" list
            _ongoingApts.push_back(apt);
            newOngoing.push_back(_ongoingApts.last());

            // Remove it from the appointment list
            it = _appointments.erase(it);
        } else {
            ++it;
        }
    }

    // Broadcast new ongoing appointments to observers
    if (newOngoing.size() > 0)
        emit newOngoingAppointments(this, newOngoing);
}

void Calendar::sendNotifications_Reminders()
{
    QDateTime now = QDateTime::currentDateTime();
    now = now.addSecs(-now.time().second());
    now = now.addMSecs(-now.time().msec());

    // Get the list of reminders that are dated at this minute and
    // erase them from reminder storage
    QLinkedList<Appointment> reminders;
    QMap<QDateTime, Appointment>::iterator it = _reminders.find(now);

    // Collect all reminders dated to now
    while (it != _reminders.end() && it.key() == now) {
        reminders.push_back(*it);
        it = _reminders.erase(it);
    }

    // Broadcast new reminders to observers
    if (reminders.count() > 0)
        emit newReminders(this, reminders);
}

void Calendar::buildCalendarImage()
{
    // TODO: we'll probably transition to a fairly neutral
    // calendar image and colorize it later.
    _image = QImage(IMAGEDIM, IMAGEDIM, QImage::Format_RGB32);
    _image.fill(_color.rgb());

    for (unsigned row = 0; row < (unsigned)IMAGEDIM; ++row) {
        QRgb* curline = (QRgb*)_image.scanLine(row);

        for (unsigned col = 0; col < (unsigned)IMAGEDIM; ++col) {
            QColor newColor = _color;
            newColor.setRed(std::min(newColor.red() + ((double)(IMAGEDIM - row)/IMAGEDIM)*75, (double)255));
            newColor.setGreen(std::min(newColor.green() + ((double)(IMAGEDIM - row)/IMAGEDIM)*75, (double)255));
            newColor.setBlue(std::min(newColor.blue() + ((double)(IMAGEDIM - row)/IMAGEDIM)*75, (double)255));

            curline[col] = newColor.rgb();
        }
    }
}

void Calendar::flushCalendarCache()
{
    _appointments.clear();
    _reminders.clear();
    _ongoingApts.clear();
}

void Calendar::parseNetworkResponse(QNetworkReply* reply)
{
    QNetworkReply::NetworkError error = reply->error();
    StatusCode oldStatus = _status;

    // Only proceed with the update if nothing went wrong.
    // Otherwise the update will be halted.
    if (error == QNetworkReply::NoError) {
        // Extract the server response and file checksum
        QString rawData = reply->readAll();
        rawData.replace("\r", "");
        rawData.replace(QRegExp("\\nDTSTAMP:[^\\n]+\\n"), "\n");

        // Calculate the calendar checksum, but exclude certain attributes
        QString checksumData = rawData;
        checksumData.replace(QRegExp("\\nTRIGGER(;|:)[^\\n]+\\n"), "\n");
        checksumData.replace(QRegExp("\\nATTENDEE(;|:)[^\\n]+\\n( [^\\n]+\\n)*"), "\n");
        int newChecksum = qChecksum(checksumData.toUtf8(), checksumData.length());
        _status = Online;

        // Compare checksums to see if a reload is necessary
        if (_calChecksum != newChecksum && rawData != "") {
            _bufferLock.lock();

#ifdef DEBUG
            // Write changed calendar to disk.
            QFile debugFile(_name + "-" + QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss"));
            if (!debugFile.open(QIODevice::WriteOnly | QIODevice::Text))
                return;
            debugFile.write(checksumData.toLocal8Bit());
            debugFile.close();
#endif

            // Flush and repopulate the cache
            flushCalendarCache();
            importCalendarData(rawData);

            // Notify the view on file changes, unless it's our initial load
            if (_calChecksum != 0)
                emit calendarChanged(this);

            _calChecksum = newChecksum;
            _bufferLock.unlock();

            // Send out our first batch of notifications
            sendNotifications();
        }
    } else {
        // In the event of a download error, set the calendar to Offline.
        _status = Offline;
        if (oldStatus == NotLoaded)
            emit formatNotRecognized(this);
    }

    // If the calendar status changed, notify observers.
    if (_status != oldStatus)
        emit statusChanged(this);
}

void Calendar::importCalendarData(QString rawData)
{
    QDateTime now = QDateTime::currentDateTime();

    // If we don't have the ICS header, this is not a valid calendar
    if (rawData.indexOf("BEGIN:VCALENDAR") != 0) {
        _name = "Invalid Calendar";
        _status = Offline;
        emit nameChanged(this);
        emit formatNotRecognized(this);
        return;
    }

    // Check if we can extract the calendar name
    int calNamePos = rawData.indexOf("X-WR-CALNAME:");
    if (calNamePos != -1) {
        _name = rawData.mid(calNamePos + 13);
        _name = _name.left(_name.indexOf("\n"));
        emit nameChanged(this);
    }

    // Loop until all events are cached
    bool eventsLeft = true;
    while (eventsLeft) {
        // Check if a new event can be extracted
        int beginPos = rawData.indexOf("BEGIN:VEVENT");
        int endPos = rawData.indexOf("END:VEVENT");

        if (beginPos == -1 || endPos == -1 || beginPos > endPos) {
            eventsLeft = false;
        } else {
            QString calInfo = rawData.mid(beginPos, endPos - beginPos);

            // Add the newly extracted appointment if it hasn't already ended
            Appointment newApt(calInfo);
            if (newApt.isValid() && now < newApt.end()) {
                _appointments.insert(newApt.start(), newApt);

                // Create reminders where needed
                int triggerPos = calInfo.indexOf("TRIGGER:-P");
                while (triggerPos != -1) {
                    QString triggerInfo = calInfo.mid(triggerPos + 10);
                    triggerInfo = triggerInfo.left(triggerInfo.indexOf("\n"));

                    // Only add reminder times that haven't passed yet
                    QDateTime reminderStamp = determineReminderStamp(newApt.start(), triggerInfo);
                    assert(reminderStamp.isValid());
                    if (now <= reminderStamp)
                        _reminders.insert(reminderStamp, newApt);

                    calInfo = calInfo.mid(triggerPos + 10);
                    triggerPos = calInfo.indexOf("TRIGGER:-P");
                }
            }

            // Prepare for next appointment
            rawData = rawData.mid(endPos + 10);
        }
    }
}

QDateTime Calendar::determineReminderStamp(const QDateTime &aptStart, const QString &triggerInfo)
{
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

short Calendar::calcTimeShift() {
    QDateTime local = QDateTime::currentDateTime();
    QDateTime utc = QDateTime::currentDateTimeUtc();

    // Calculate the hour difference between this timezone and UTC
    if (local.date() == utc.date())
        return local.time().hour() - utc.time().hour();
    else if (local.date() < utc.date())
        return -(utc.time().hour() + (24 - local.time().hour()));
    else if (utc.date() < local.date())
        return local.time().hour() + (24 - utc.time().hour());
    else {
        abort();
        return 0;
    }
}
