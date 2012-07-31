#ifndef APTCACHE_H
#define APTCACHE_H

#include <QMutex>
#include <QMultiMap>
#include <QDateTime>
#include <QLinkedList>
#include "appointment.h"

/**
  * Holds a calendar's imported appointment lists. Includes reminders,
  * ongoing appointments and future appointments. This class is NOT thread-safe.
  *
  * \author Pieter De Decker
  */
class AptCache
{
public:
    AptCache();

    // Getters
    QMultiMap<QDateTime, Appointment>* appointments() { return &_appointments; }
    QMultiMap<QDateTime, Appointment>* reminders() { return &_reminders; }
    QLinkedList<Appointment>* ongoingApts() { return &_ongoingApts; }

    /** Updates the list of ongoing appointments for the current timestamp. Returns a list
      * of newly ongoing appointments. */
    QLinkedList<Appointment> updateOngoingApts();
private:
    /** [HELPER] Removes ongoing appointments that have expired. */
    void updateOngoingApts_RemoveExpired(QLinkedList<Appointment>& allOngoing, const QDateTime& now);

    /** [HELPER] Import newly ongoing appointments to apts and return them as a linked list. */
    QLinkedList<Appointment> updateOngoingApts_CollectNewlyOngoing(QLinkedList<Appointment>& allOngoing, const QDateTime& now);

    /** Contains all appointments that aren't ongoing. Sorted on start time. */
    QMultiMap<QDateTime, Appointment> _appointments;

    /** Contains all non-expired reminders with their requested alarm time. */
    QMultiMap<QDateTime, Appointment> _reminders;

    /** Contains all ongoing appointments. */
    QLinkedList<Appointment> _ongoingApts;
};

#endif // APTCACHE_H
