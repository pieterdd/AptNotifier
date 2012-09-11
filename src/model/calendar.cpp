#include "calendar.h"

#include "aptcache.h"
#include "icsparser.h"
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

Calendar::Calendar(const QString &url, const QColor &color) {
    QByteArray urlArray;
    _url = QUrl::fromEncoded(urlArray.append(url));
    _name = "Untitled Calendar";
    _calChecksum = 0;
    _color = color;
    _status = NotLoaded;
    _aptCache = new AptCache();
    buildCalendarImage();

    // Wire QObjects
    connect(&_httpDl, SIGNAL(receivedData(bool,QString*)), this, SLOT(parseNetworkResponse(bool,QString*)));
    connect(&_nfyTimer, SIGNAL(timeout()), this, SLOT(sendNotifications()));
    _nfyTimer.setInterval(30000);
    _nfyTimer.setSingleShot(true);
}

Calendar::~Calendar() {
    delete _aptCache;
}

void Calendar::update()
{
    // Start calendar download asynchronously. After retrieving the
    // file, parseNetworkResponse will take over.
    _httpDl.doGet(_url);
    Logger::instance()->add(CLASSNAME, this, "Update request was filed");
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
    Logger::instance()->add(CLASSNAME, this, "Checking notifications at "
                            + QDateTime::currentDateTime().toString("hh:mm:ss") + "...");

    // First get the ongoing appointment notifications out the door,
    // then process the reminders.
    sendNotifications_Ongoing();
    sendNotifications_Reminders();

    // Check notifications again in half a minute
    engageBufferLock("accessing timer");
    _nfyTimer.start();
    releaseBufferLock("finished accessing timer");
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

void Calendar::repopulateCache(const ICSParser& parser) {
    AptCache* aptCache = parser.readAppointments();
    assert(aptCache);

    // Replace old cache, update checksum and update name
    engageBufferLock("replacing old appointment cache");
    delete _aptCache;
    _aptCache = aptCache;
    _calChecksum = parser.checksum();
    releaseBufferLock("installed new appointment cache");

    // Update other attributes that will trigger update signals
    if (name() != parser.name())
        setName(parser.name());
    setStatus(Online);
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

void Calendar::parseNetworkResponse(bool success, QString *data) {
    assert(data);
    engageBufferLock("accessing status and timer");
    StatusCode oldStatus = _status;
    _nfyTimer.stop();
    releaseBufferLock("releasing status and timer");

    if (!success) {
        // In the event of a download error, set the calendar to Offline.
        Logger::instance()->add(CLASSNAME, this, "Error fetching update");

        setStatus(Offline);
        if (oldStatus != Offline)
            emit formatNotRecognized(this);
    } else {
        Logger::instance()->add(CLASSNAME, this, "Parsing ICS data...");
        ICSParser parser(*data);

        // Check ICS validity first
        if (!parser.holdsValidICS()) {
            Logger::instance()->add(CLASSNAME, this, "Downloaded data appears to be invalid ICS");
            setStatus(Offline);
        } else {
            // Only repopulate the AptCache if the calendar changed
            if (calChecksum() != parser.checksum())
                repopulateCache(parser);
            if (status() == Online) {
                // This will re-enable the timer and, if the cache was repopulated,
                // trigger the first batch of notifications.
                sendNotifications();
            }
        }
    }

    Logger::instance()->add(CLASSNAME, this, "Finished updating");
}

void Calendar::parseNetworkResponse_Fail() {
    Logger::instance()->add(CLASSNAME, this, "Something went wrong while fetching an update");
    emit formatNotRecognized(this);
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
    Q_UNUSED(reason)
#ifdef VERBOSELOCKING
    Logger::instance()->add(CLASSNAME, this, "Requesting buffer lock with reason '" + reason + "'");
#endif
    _bufferLock.lock();
#ifdef VERBOSELOCKING
    Logger::instance()->add(CLASSNAME, this, "Activated buffer lock with reason '" + reason + "'");
#endif
}

void Calendar::releaseBufferLock(const QString& reason = "no reason given")
{
    Q_UNUSED(reason)
    _bufferLock.unlock();
#ifdef VERBOSELOCKING
    Logger::instance()->add(CLASSNAME, this, "Released buffer lock with reason '" + reason + "'");
#endif
}
