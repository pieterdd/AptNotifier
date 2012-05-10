#ifndef APPOINTMENTLIST_H
#define APPOINTMENTLIST_H

#include "infowidget.h"
#include "model/calendar.h"
#include "model/appointment.h"
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QLinkedList>
#include <QHBoxLayout>
#include <QVBoxLayout>

/**
  * Widget that displays appointments that is to be
  * used inside a toaster.
  * \author Pieter De Decker
  */
class AppointmentList : public InfoWidget
{
    Q_OBJECT
public:
    AppointmentList(Calendar* cal, const QLinkedList<Appointment>& aptList);

    /** Activates the slide cycle timer. */
    void start();
private slots:
    // Advances to the next appointment
    void nextSlide();
private:
    /** Constructs GUI, wires components. */
    void setupGUI();

    // Layout
    QVBoxLayout _vlMain;
    QHBoxLayout _hlBottom;

    // Controls
    QLabel _lblAppointment;
    QLabel _lblCalName;
    QPushButton _btnNext;

    // Data
    Calendar* _cal;
    QLinkedList<Appointment> _aptList;

    // Notification cycling timer
    QTimer _nfyTimer;
    QLinkedList<Appointment>::iterator _nextAptIt;
};

#endif // APPOINTMENTLIST_H