#include "calendar.h"

#include "aptcache.h"
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
const char* Calendar::CLASSNAME = "Calendar";

Calendar::Calendar(const QString &url, const QColor &color)
    : _naMgr(this)
{
    _url = QUrl(url);
    _name = "Untitled Calendar";
    _calChecksum = 0;
    _color = color;
    _status = NotLoaded;
    _aptCache = new AptCache();
    _naReply = NULL;
    buildCalendarImage();

    // Wire QObjects
    connect(&_httpDl, SIGNAL(receivedData(bool,QString*)), this, SLOT(parseNetworkResponse_NEW(bool,QString*)));
    connect(&_nfyTimer, SIGNAL(timeout()), this, SLOT(sendNotifications()));
}

Calendar::~Calendar() {
    _naReply->deleteLater();
    delete _aptCache;
}

void Calendar::update()
{
    _bufferLock.lock();
    QNetworkReply* reply = _naReply;
    _bufferLock.unlock();

    // Don't update if an active request exists
    if (reply) {
        Logger::instance()->add(CLASSNAME, "Skipping update for 0x" + QString::number((unsigned)this, 16) + " since an update is already in progress.");
        return;
    }

    // Start calendar download asynchronously. After retrieving the
    // file, parseNetworkResponse will take over.
    Logger::instance()->add(CLASSNAME, "Fetching update for 0x" + QString::number((unsigned)this, 16) + "...");
    _httpDl.doGet(_url);
    Logger::instance()->add(CLASSNAME, "Update request for 0x" + QString::number((unsigned)this, 16) + " was filed.");
}

void Calendar::drawBorder(QImage &img, int thickness, const QColor &color) {
    assert(thickness < img.width() && thickness < img.height());
    fillRectangle(img, 0, 0, img.width(), thickness, color);                               // Top
    fillRectangle(img, img.width() - thickness, 0, thickness, img.height(), color);        // Right
    fillRectangle(img, 0, img.height() - thickness, img.width(), thickness, color);        // Bottom
    fillRectangle(img, 0, 0, thickness, img.height(), color);                              // Left
}

void Calendar::sendNotifications()
{
    // First get the ongoing appointment notifications out the door,
    // then process the reminders.
    sendNotifications_Ongoing();
    sendNotifications_Reminders();

    // Check notifications again in half a minute
    QTimer::singleShot(30000, this, SLOT(sendNotifications()));
}

void Calendar::sendNotifications_Ongoing()
{
    // Update the list of ongoing appointments
    engageBufferLock("updating ongoing appointment list");
    QLinkedList<Appointment> newOngoing = _aptCache->updateOngoingApts();
    releaseBufferLock("updated ongoing appointment list");

    // Broadcast new ongoing appointments to observers
    if (newOngoing.size() > 0) {
        emit newOngoingAppointments(this, newOngoing);
    }
}

void Calendar::sendNotifications_Reminders()
{
    QDateTime now = QDateTime::currentDateTime();
    now = now.addSecs(-now.time().second());
    now = now.addMSecs(-now.time().msec());

    // Get the list of reminders that are dated at this minute and
    // erase them from reminder storage
    QLinkedList<Appointment> reminders;
    engageBufferLock("accessing/updating reminders list");
    QMap<QDateTime, Appointment>::iterator it = _aptCache->reminders()->find(now);

    // Collect all reminders dated to now
    while (it != _aptCache->reminders()->end() && it.key() == now) {
        reminders.push_back(*it);
        it = _aptCache->reminders()->erase(it);
    }
    releaseBufferLock("finished accessing reminders list");

    // Broadcast new reminders to observers
    if (reminders.count() > 0)
        emit newReminders(this, reminders);
}

int Calendar::calChecksum() {
    int retVal;
    engageBufferLock("accessing calendar checksum");
    retVal = _calChecksum;
    releaseBufferLock("accessed calendar checksum");

    return retVal;
}

void Calendar::setCalChecksum(int calChecksum) {
    engageBufferLock("updating calendar checksum");
    _calChecksum = calChecksum;
    releaseBufferLock("updated calendar checksum");
}

void Calendar::setName(const QString& name) {
    engageBufferLock("updating name to " + name);
    _name = name;
    releaseBufferLock("updated name");
    emit nameChanged(this);
}

void Calendar::setStatus(StatusCode status) {
    engageBufferLock("updating status to " + status);
    _status = status;
    releaseBufferLock("updated status");
    emit statusChanged(this);
}

QString& operator+(QString& str, Calendar& cal) {
    if (cal._status == Calendar::NotLoaded)
        str += cal._url.toString();
    else
        str += cal._name;
    return str;
}

void Calendar::buildCalendarImage()
{
    _image = QImage(IMAGEDIM, IMAGEDIM, QImage::Format_RGB32);
    _image.fill(_color.rgb());

    // Create gradient
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

void Calendar::fillRectangle(QImage &img, unsigned x, unsigned y, unsigned w, unsigned h, const QColor& color) {
    for (unsigned row = y; row < y + h; ++row) {
        QRgb* curline = (QRgb*)img.scanLine(row);

        for (unsigned col = x; col < x + w; ++col)
            curline[col] = color.rgb();
    }
}

void Calendar::parseNetworkResponse_NEW(bool success, QString *data) {
    // Retrieve the current calendar status in a thread-safe way
    Logger::instance()->add(CLASSNAME, "Fetching current status for 0x" + QString::number((unsigned)this, 16) + "...");
    StatusCode oldStatus = status();
    Logger::instance()->add(CLASSNAME, "Fetched current status for 0x" + QString::number((unsigned)this, 16) + ".");

    if (!success) {
        // In the event of a download error, set the calendar to Offline.
        Logger::instance()->add(CLASSNAME, "Error fetching update for 0x" + QString::number((unsigned)this, 16) + ".");
        setStatus(Offline);
        if (oldStatus == NotLoaded)
            emit formatNotRecognized(this);
    } else {
        QString rawData = *data;
        rawData.replace("\r", "");

        // Calculate the calendar checksum based on Last Modified attributes
        QString checksumData = rawData;
        checksumData.replace(QRegExp("((^|\\n)(?!LAST-MODIFIED)[^\\n]*)+"), "");
        int newChecksum = qChecksum(checksumData.toUtf8(), checksumData.length());
        setStatus(Online);

        Logger::instance()->add(CLASSNAME, "Comparing update checksum for 0x" + QString::number((unsigned)this, 16) + "...");

        // If we can't checksum because Last Modified attributes are unavailable,
        // fall back on regular file checksumming.
        if (newChecksum == 0)
            newChecksum = qChecksum(rawData.toUtf8(), rawData.length());

        // Compare checksums to see if a reload is necessary
        if (calChecksum() != newChecksum && rawData != "") {
            Logger::instance()->add(CLASSNAME, "Change was detected for 0x" + QString::number((int)this, 16) + ".");

            // Construct our new appointment cache off the raw data. Then replace the old cache.
            AptCache* aptCache = parseICSFile(rawData);
            if (!aptCache)
                return;

            engageBufferLock("replacing old appointment cache");
            delete _aptCache;
            _aptCache = aptCache;
            releaseBufferLock("installed new appointment cache");

            // Notify the view on file changes, unless it's our initial load
            if (calChecksum() != 0)
                emit calendarChanged(this);
            setCalChecksum(newChecksum);

            // Send out our first batch of notifications (thread-safe)
            sendNotifications();
        }
    }

    Logger::instance()->add(CLASSNAME, "Finished updating 0x" + QString::number((unsigned)this, 16) + ".");
}

void Calendar::parseNetworkResponse() {
    _bufferLock.lock();
    assert(_naReply);
    QNetworkReply* reply = _naReply;
    _bufferLock.unlock();

    // TODO: we suspect that sometimes a SIGSEGV occurs within the bounds
    // of this function. We'll remove the excessive log calls when we've
    // successfully tracked down the problem.

    // Retrieve the current calendar status in a thread-safe way
    Logger::instance()->add(CLASSNAME, "Fetching current status for 0x" + QString::number((unsigned)this, 16) + "...");
    StatusCode oldStatus = status();
    Logger::instance()->add(CLASSNAME, "Fetched current status for 0x" + QString::number((unsigned)this, 16) + ".");
    QNetworkReply::NetworkError error = reply->error();

    // Only proceed with the update if nothing went wrong.
    // Otherwise the update will be halted.
    if (error == QNetworkReply::NoError) {
        // Extract the server response and file checksum
        QString rawData = reply->readAll();
        rawData.replace("\r", "");

        // Calculate the calendar checksum based on Last Modified attributes
        QString checksumData = rawData;
        checksumData.replace(QRegExp("((^|\\n)(?!LAST-MODIFIED)[^\\n]*)+"), "");
        int newChecksum = qChecksum(checksumData.toUtf8(), checksumData.length());
        setStatus(Online);

        Logger::instance()->add(CLASSNAME, "Comparing update checksum for 0x" + QString::number((unsigned)this, 16) + "...");

        // If we can't checksum because Last Modified attributes are unavailable,
        // fall back on regular file checksumming.
        if (newChecksum == 0)
            newChecksum = qChecksum(rawData.toUtf8(), rawData.length());

        // Compare checksums to see if a reload is necessary
        if (calChecksum() != newChecksum && rawData != "") {
            Logger::instance()->add(CLASSNAME, "Change was detected for 0x" + QString::number((int)this, 16) + ".");

            // Construct our new appointment cache off the raw data. Then replace the old cache.
            AptCache* aptCache = parseICSFile(rawData);
            if (!aptCache)
                return;

            engageBufferLock("replacing old appointment cache");
            delete _aptCache;
            _aptCache = aptCache;
            releaseBufferLock("installed new appointment cache");

            // Notify the view on file changes, unless it's our initial load
            if (calChecksum() != 0)
                emit calendarChanged(this);
            setCalChecksum(newChecksum);

            // Send out our first batch of notifications (thread-safe)
            sendNotifications();
        }
    } else {
        // In the event of a download error, set the calendar to Offline.
        Logger::instance()->add(CLASSNAME, "Error fetching update for 0x" + QString::number((unsigned)this, 16) + ".");
        setStatus(Offline);
        if (oldStatus == NotLoaded)
            emit formatNotRecognized(this);
    }

    Logger::instance()->add(CLASSNAME, "Finished updating 0x" + QString::number((unsigned)this, 16) + ".");

    // Dispose update object
    _bufferLock.lock();
    _naReply->deleteLater();
    _naReply = NULL;
    _bufferLock.unlock();
}

void Calendar::parseNetworkResponse_Fail() {
    Logger::instance()->add(CLASSNAME, "Something went wrong while fetching an update for 0x" + QString::number((unsigned)this, 16) + ".");
    emit formatNotRecognized(this);
}

AptCache* Calendar::parseICSFile(QString rawData) {
    Logger::instance()->add(CLASSNAME, "Importing calendar changes for 0x" + QString::number((int)this, 16) + ".");

    // If we don't have the ICS header, this is not a valid calendar
    if (rawData.indexOf("BEGIN:VCALENDAR") != 0) {
        Logger::instance()->add(CLASSNAME, "Import of 0x" + QString::number((int)this, 16) + " failed because it" +
                                " isn't recognized as an ICS file.");
        setName("Invalid Calendar");
        setStatus(Offline);
        emit formatNotRecognized(this);

        return NULL;
    }

    // Check if we can extract the calendar name
    int calNamePos = rawData.indexOf("X-WR-CALNAME:");
    if (calNamePos != -1) {
        QString name;
        name = rawData.mid(calNamePos + 13);
        setName(name.left(name.indexOf("\n")));
    }

    // We've done our initial checks. Now let's fill that cache already!
    AptCache* aptCache = new AptCache();
    parseICSFile_FillAptCache(rawData, aptCache);

    return aptCache;
}

void Calendar::parseICSFile_FillAptCache(QString &rawData, AptCache *aptCache) {
    QDateTime now = QDateTime::currentDateTime();

    // Loop until all events are cached
    while (parseICSFile_EventsLeft(rawData)) {
        QString calInfo = parseICSFile_GetRawApt(rawData);
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
                QDateTime reminderStamp = determineReminderStamp(newApt.start(), triggerInfo);
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
}

bool Calendar::parseICSFile_EventsLeft(const QString& rawData) const {
    int beginPos = rawData.indexOf("BEGIN:VEVENT");
    int endPos = rawData.indexOf("END:VEVENT");

    if (beginPos == -1 || endPos == -1 || beginPos > endPos)
        return false;
    else
        return true;
}

QString Calendar::parseICSFile_GetRawApt(const QString& rawData) const {
    int beginPos = rawData.indexOf("BEGIN:VEVENT");
    int endPos = rawData.indexOf("END:VEVENT");
    QString calInfo = rawData.mid(beginPos, endPos - beginPos);

    return calInfo;
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

void Calendar::engageBufferLock(const QString& reason = "no reason given")
{
#ifdef VERBOSELOCKING
    Logger::instance()->add(CLASSNAME, "Requesting buffer lock for object 0x" + QString::number((unsigned)this, 16) + " (" + reason + ")");
#endif
    _bufferLock.lock();
#ifdef VERBOSELOCKING
    Logger::instance()->add(CLASSNAME, "Activated buffer lock for object 0x" + QString::number((unsigned)this, 16) + " (" + reason + ")");
#endif
}

void Calendar::releaseBufferLock(const QString &reason)
{
    _bufferLock.unlock();
#ifdef VERBOSELOCKING
    Logger::instance()->add(CLASSNAME, "Released buffer lock for object 0x" + QString::number((unsigned)this, 16) + " (" + reason + ")");
#endif
}
