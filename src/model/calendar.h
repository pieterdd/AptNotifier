#ifndef CALENDAR_H
#define CALENDAR_H

#include "logger.h"
#include <QUrl>
#include <QColor>
#include <QTimer>
#include <QImage>
#include <QMutex>
#include <QDebug>
#include <QString>
#include <QObject>
#include <QMetaType>
#include <QDateTime>
#include <QMultiMap>
#include <QLinkedList>
#include <QNetworkAccessManager>

class AptCache;
class Appointment;
class QNetworkReply;
class QNetworkAccessManager;

/**
  * Represents an online iCal calendar.
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
    QString name() {     // This getter requires thread sync
        engageBufferLock("getting name attribute");
        QString retVal = _name;
        releaseBufferLock("got name attribute");
        return retVal;
    }
    const QColor& color() const { return _color; }
    const QImage& image() const { return _image; }
    static short getTimeShift() { return Calendar::timeShift; }
    StatusCode status() {       // This getter requires thread sync
        engageBufferLock("getting status attribute");
        StatusCode retVal = _status;
        releaseBufferLock("got status attribute");
        return retVal;
    }

    /** [THREAD-SAFE] Triggers a refresh of the calendar. */
    void update();

    /** This function is used by view classes to draw a border around
      * a calendar image. Since this image is resized often, we have
      * to add the border right before showing the image on screen.
      */
    static void drawBorder(QImage& img, int thickness, const QColor& color);

    /** [THREAD-SAFE] String representation generator. Do not use this internally where
      * _bufferLock has been engaged, this will block the program! QString concatenation
      * will not engage _bufferLock and can be used internally. */
    QString toString() {
        QString retVal;

        engageBufferLock("making string representation");
        operator +(retVal, *this);
        releaseBufferLock("made string representation");

        return retVal;
    }
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
    static const char* CLASSNAME;

    /** [THREAD-SAFE] Getter for the calendar checksum. */
    int calChecksum();

    /** [THREAD-SAFE] Setter for the calendar checksum. */
    void setCalChecksum(int calChecksum);

    /** [THREAD-SAFE] Setter for the calendar name. Notifies observers afterwards. */
    void setName(const QString& name);

    /** [THREAD-SAFE] Setter for the status attribute. Notifies observers afterwards. */
    void setStatus(StatusCode status);

    /** Thread-unsafe QString concatenator for Calendar. To be used internally with appropriate thread
      * synchronization only. */
    friend QString& operator+(QString& str, Calendar& cal);

    /** Creates the pixmap for this calendar based on the color. */
    void buildCalendarImage();

    /** Fills a rectangle area in an image with a color. */
    static void fillRectangle(QImage& img, unsigned x, unsigned y, unsigned w, unsigned h, const QColor& color);

    /** Given a raw ICS file, this function parses its contents and loads all relevant content into a new
      * AptCache object. Ownership of this object is transferred over to the caller. May return NULL if
      * import failed. */
    AptCache* parseICSFile(QString rawData);

    /** [HELPER] Loops over each VEVENT in rawData, adding new appointments to aptCache as they pop up. */
    void parseICSFile_FillAptCache(QString& rawData, AptCache* aptCache);

    /** [HELPER] Checks if at least one more VEVENT can be extracted from rawData. */
    bool parseICSFile_EventsLeft(const QString& rawData) const;

    /** [HELPER] Extracts the raw text of the next VEVENT from rawData. */
    QString parseICSFile_GetRawApt(const QString& rawData) const;

    /** Determines the reminder timestamp of an appointment based on the appointment
      * time and the "TRIGGER" field. */
    QDateTime determineReminderStamp(const QDateTime& aptStart, const QString& triggerInfo);

    /** Calculates the user's time zone shift compared to UTC.
      * \todo This will likely cause a bug when used in Newfoundland or other places
      * that have non-integer timezone shifts. */
    static short calcTimeShift();

    /** [THREAD-SAFE] Helper: engages _bufferLock and writes status info about this to the log. */
    void engageBufferLock(const QString& reason);

    /** [THREAD-SAFE] Helper: releases _bufferLock and writes status info about this to the log. */
    void releaseBufferLock(const QString& reason);

    QUrl _url;        /* Should not change after the constructor finishes! */
    QString _name;
    QColor _color;
    QImage _image;
    QTimer _nfyTimer;
    QNetworkAccessManager _naMgr;

    /** Holds a hash that helps detect changes in new calendars. _bufferLock required for access. */
    int _calChecksum;

    /** Represents the current state of the calendar. _bufferLock required for access. */
    enum StatusCode _status;

    /** Contains all ongoing appointments, future appointments and
      * scheduled reminders. _bufferLock required for access. */
    AptCache* _aptCache;

    /** Mutex for _aptCache, _status and _calChecksum. */
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
