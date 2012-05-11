#ifndef CALENDAR_H
#define CALENDAR_H

#include <QUrl>
#include <QColor>
#include <QTimer>
#include <QImage>
#include <QMutex>
#include <QString>
#include <QObject>
#include <QMetaType>
#include <QDateTime>
#include <QMultiMap>
#include <QLinkedList>
#include <QNetworkAccessManager>

class Appointment;
class QNetworkReply;
class QNetworkAccessManager;

/**
  * Stores a list of appointments.
  * \author Pieter De Decker
  */
class Calendar : public QObject
{
    Q_OBJECT
    Q_ENUMS(ExceptionCode)
public:
    Calendar(const QString& url, const QColor& color);
    ~Calendar();

    /** STATUS CODES FOR CALENDARS
      *
      * NotLoaded
      *     We haven't attempted to fetch the calendar yet.
      * Online
      *     Last automatic/manual update was successful.
      * Offline
      *     Last automatic/manual update was unsuccessful.
      */
    enum StatusCode { NotLoaded, Online, Offline };

    /* Getters */
    QString url() const { return (QString)_url.toEncoded(); }
    const QString& name() const { return _name; }
    const QColor& color() const { return _color; }
    const QImage& image() const { return _image; }
    static short getTimeShift() { return Calendar::timeShift; }
    StatusCode status() const { return _status; }

    /** [THREAD-SAFE] Triggers a refresh of the calendar. */
    void update();

    /** This function is used by view classes to draw a border around
      * a calendar image. Since this image is resized often, we have
      * to add the border right before showing the image on screen.
      */
    static void drawBorder(QImage& img, int thickness, const QColor& color);
private slots:
    /** [THREAD-SAFE] Analyses the downloaded calendar file and rebuilds the cache if the
      * checksum of the file has changed. */
    void parseNetworkResponse(QNetworkReply*);

    /** [THREAD-SAFE] On regular intervals, this function is
      * called to trigger notifications about ongoing appointments
      * and reminders. */
    void sendNotifications();
    void sendNotifications_Ongoing();
    void sendNotifications_Reminders();
private:
    /** Creates the pixmap for this calendar based on the color. */
    void buildCalendarImage();

    /** Fills a rectangle area in an image with a color. */
    static void fillRectangle(QImage& img, unsigned x, unsigned y, unsigned w, unsigned h, const QColor& color);

    /** Clears all appointments and reminders. */
    void flushCalendarCache();

    /** Parses an ICS file and extracts appointments with their reminders. */
    void importCalendarData(QString rawData);

    /** Determines the reminder timestamp of an appointment based on the appointment
      * time and the "TRIGGER" field. */
    QDateTime determineReminderStamp(const QDateTime& aptStart, const QString& triggerInfo);

    /** Calculates the user's time zone shift compared to UTC.
      * \todo This will likely cause a bug when used in Newfoundland or other places
      * that have non-integer timezone shifts. */
    static short calcTimeShift();

    QUrl _url;
    QString _name;
    int _calChecksum;
    QColor _color;
    QImage _image;
    QTimer _nfyTimer;
    QNetworkAccessManager _naMgr;
    enum StatusCode _status;

    /** Contains all appointments that aren't ongoing. Sorted on start time. */
    QMultiMap<QDateTime, Appointment> _appointments;

    /** Contains all non-expired reminders with their requested alarm time. */
    QMultiMap<QDateTime, Appointment> _reminders;

    /** Contains all ongoing appointments. */
    QLinkedList<Appointment> _ongoingApts;

    /** Mutex for all the aforementioned buffers. */
    QMutex _bufferLock;

    static short timeShift;
    static const short IMAGEDIM;
signals:
    /** Broadcast when the downloaded calendar has an unrecognized format. */
    void formatNotRecognized(Calendar*);

    /** Broadcast when the calendar's status code changes. Useful for
      * providing availability updates to the view. */
    void statusChanged(Calendar*);

    /** Broadcast when the calendar name changes. */
    void nameChanged(Calendar*);

    /** Broadcasts new ongoing appointments at semi-regular intervals. */
    void newOngoingAppointments(Calendar*, const QLinkedList<Appointment>&);

    /** Broadcasts reminders at semi-regular intervals. */
    void newReminders(Calendar*, const QLinkedList<Appointment>&);

    /** Broadcast when the calendar file has new changes and will
      * be re-parsed. */
    void calendarChanged(Calendar*);
};

Q_DECLARE_METATYPE(Calendar*)

#endif // CALENDAR_H
