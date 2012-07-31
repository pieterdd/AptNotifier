#include "aptcache.h"

AptCache::AptCache()
{
}

QLinkedList<Appointment> AptCache::updateOngoingApts() {
    QDateTime now = QDateTime::currentDateTime();
    now = now.addSecs(-now.time().second());

    // Remove appointments that are no longer ongoing
    updateOngoingApts_RemoveExpired(_ongoingApts, now);

    // Collect newly ongoing appointments and transfer them to a separate list
    QLinkedList<Appointment> newOngoing = updateOngoingApts_CollectNewlyOngoing(_ongoingApts, now);

    return newOngoing;
}

void AptCache::updateOngoingApts_RemoveExpired(QLinkedList<Appointment>& allOngoing, const QDateTime& now) {
    for (QLinkedList<Appointment>::iterator it = allOngoing.begin(); it != allOngoing.end(); ++it) {
        const Appointment& apt = *it;

        if (apt.end() < now)
            it = allOngoing.erase(it);
    }
}

QLinkedList<Appointment> AptCache::updateOngoingApts_CollectNewlyOngoing(QLinkedList<Appointment>& allOngoing, const QDateTime& now) {
    QLinkedList<Appointment> newOngoing;
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
            allOngoing.push_back(apt);
            newOngoing.push_back(allOngoing.last());

            // Remove it from the appointment list
            it = _appointments.erase(it);
        } else {
            ++it;
        }
    }

    return newOngoing;
}
