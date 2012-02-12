#include "calendar.h"

#include "appointment.h"
#include <QFile>
#include <cassert>
#include <QMessageBox>
#include <QNetworkReply>
#include <QNetworkRequest>

short Calendar::timeShift = Calendar::calcTimeShift();

Calendar::Calendar(const QString &url, const QColor &color)
    : _naMgr(this)
{
    QByteArray qbaUrl;
    _url = QUrl::fromEncoded(qbaUrl.append(url));
    _name = "Untitled Calendar";
    _color = color;
    buildCalendarPixmap();

    // Wire timers
    connect(&_nfyTimer, SIGNAL(timeout()), this, SLOT(prepareNotifications()));

    // Start calendar download
    connect(&_naMgr, SIGNAL(finished(QNetworkReply*)), this, SLOT(buildCalendar(QNetworkReply*)));
    _naMgr.get(QNetworkRequest(_url));
}

Calendar::~Calendar() {
}

void Calendar::prepareNotifications()
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
    do {
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
    } while (it != _appointments.end() && !foundAllNewlyOngoing);

    // Broadcast new ongoing appointments to observers and schedule a new update
    if (newOngoing.size() > 0)
        emit newOngoingAppointments(this, newOngoing);
    QTimer::singleShot(30000, this, SLOT(prepareNotifications()));
}

void Calendar::buildCalendarPixmap()
{
    // TODO: make it pretty
    _pixmap = QPixmap(64, 64);
    _pixmap.fill(_color);
}

void Calendar::buildCalendar(QNetworkReply* reply)
{
    QString rawData = reply->readAll();

    // If we don't have the ICS header, this is not a valid calendar
    if (rawData.indexOf("BEGIN:VCALENDAR") != 0) {
        _name = "Invalid Calendar";
        emit nameChanged(this);
        emit calendarExceptionThrown(this, InvalidFormat);
        return;
    }

    // Check if we can extract the calendar name
    int calNamePos = rawData.indexOf("X-WR-CALNAME:");
    if (calNamePos != -1) {
        _name = rawData.mid(calNamePos + 13);
        _name = _name.left(_name.indexOf("\r\n"));
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
            // Create the new appointment
            Appointment newApt(rawData.mid(beginPos, endPos - beginPos));
            if (newApt.isValid())
                _appointments.insert(newApt.start(), newApt);

            // Prepare for next appointment
            rawData = rawData.mid(endPos + 10);
        }
    }

    // Send out our first batch of notifications
    prepareNotifications();
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
    else
        abort();
}
