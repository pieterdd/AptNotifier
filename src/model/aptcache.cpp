#include "aptcache.h"

AptCache::AptCache()
{
}

QList<Appointment> AptCache::updateOngoingApts() {
    QDateTime now = QDateTime::currentDateTime();
    now = now.addSecs(-now.time().second());

    // Remove appointments that are no longer ongoing
    updateOngoingApts_RemoveExpired(_ongoingApts, now);

    // Collect newly ongoing appointments and transfer them to a separate list
    QList<Appointment> newOngoing = updateOngoingApts_CollectNewlyOngoing(_ongoingApts, now);

    return newOngoing;
}

void AptCache::updateOngoingApts_RemoveExpired(QList<Appointment> &allOngoing, const QDateTime& now) {
    for (QList<Appointment>::iterator it = allOngoing.begin(); it != allOngoing.end(); ++it) {
        const Appointment& apt = *it;

        if (apt.end() < now)
            it = allOngoing.erase(it);
    }
}

QList<Appointment> AptCache::updateOngoingApts_CollectNewlyOngoing(QList<Appointment> &allOngoing, const QDateTime& now) {
    QList<Appointment> newOngoing;
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
