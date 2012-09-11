#ifndef APPOINTMENTBUNDLE_H
#define APPOINTMENTBUNDLE_H

#include "model/appointment.h"
#include <QString>
#include <QLinkedList>
class Calendar;

/**
  * Structure that assists in sending lists of
  * "Event Reminder" or "Happening Now" appointments
  * to an active toaster.
  * \author Pieter De Decker
  */
class AptBundle
{
public:
    AptBundle(Calendar* cal, const QString& title, const QList<Appointment>& list);

    /* Getters */
    Calendar* cal() const { return _cal; }
    const QString& title() const { return _title; }
    const QList<Appointment>& list() const { return _list; }
private:
    Calendar* _cal;
    QString _title;
    QList<Appointment> _list;
};

#endif // APPOINTMENTBUNDLE_H
