/**
 * \class Calendar
 * \author Pieter De Decker
 *
 * \brief Stores a list of appointments.
 */

#ifndef CALENDAR_H
#define CALENDAR_H

#include <QUrl>
#include <QColor>
#include <QTimer>
#include <QMutex>
#include <QString>
#include <QObject>
#include <QPixmap>
#include <QDateTime>
#include <QMultiMap>
#include <QLinkedList>
#include <QNetworkAccessManager>

class Appointment;
class QNetworkReply;
class QNetworkAccessManager;

class Calendar : public QObject
{
    Q_OBJECT
    Q_ENUMS(ExceptionType)
public:
    Calendar(const QString& url, const QColor& color);
    ~Calendar();

    /* Getters */
    QString url() const { return (QString)_url.toEncoded(); }
    const QString& name() const { return _name; }
    const QColor& color() const { return _color; }
    const QPixmap& pixmap() const { return _pixmap; }
    static short getTimeShift() { return Calendar::timeShift; }

    /**
      * ERROR CODES FOR CALENDAR HANDLING
      *
      * InvalidFormat
      *     We couldn't recognize this as a valid calendar.
      */
    enum ExceptionType { InvalidFormat };
private slots:
    /** Parses a calendar file to extract all appointments */
    void buildCalendar(QNetworkReply*);

    /** On regular intervals, this function is called to trigger notifications
      * about ongoing appointments and reminders */
    void prepareNotifications();
private:
    /** Creates the pixmap for this calendar based on the color. */
    void buildCalendarPixmap();

    /** Calculates the user's time zone shift compared to UTC.
      * \todo This will likely cause a bug when used in Newfoundland or other places
      * that have non-integer timezone shifts. */
    static short calcTimeShift();

    QUrl _url;
    QString _name;
    QColor _color;
    QPixmap _pixmap;
    QTimer _nfyTimer;
    QNetworkAccessManager _naMgr;

    /** Contains all appointments that aren't ongoing. Sorted on start time. */
    QMultiMap<QDateTime, Appointment> _appointments;

    /** Contains all ongoing appointments. */
    QLinkedList<Appointment> _ongoingApts;

    static short timeShift;
signals:
    /** Broadcast for calendar exceptions. We picked this over C++
      * exception throwing because it works asynchronously as well. */
    void calendarExceptionThrown(Calendar*, Calendar::ExceptionType);

    /** Broadcast when the calendar name changes. */
    void nameChanged(Calendar*);

    /** Broadcasts new ongoing appointments at semi-regular intervals. */
    void newOngoingAppointments(Calendar*, const QLinkedList<Appointment>& list);
};

#endif // CALENDAR_H
