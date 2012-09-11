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
    // Returns to the previous appointment
    void prevSlide();

    // Advances to the next appointment
    void nextSlide();
private:
    /** Constructs GUI, wires components. */
    void setupGUI(Calendar* cal);

    /** Loads a certain appointment into the GUI. */
    void loadAppointment(QLinkedList<Appointment>::iterator it);

    // Layout
    QVBoxLayout _vlMain;
    QHBoxLayout _hlCal;
    QHBoxLayout _hlBottom;

    // Controls
    QLabel _lblAppointment;
    QLabel _lblCalImg;
    QLabel _lblCalName;
    QLabel _lblCounter;
    QPushButton _btnBack;
    QPushButton _btnNext;

    // Data
    QLinkedList<Appointment> _aptList;

    // Notification cycling timer
    QTimer _nfyTimer;
    QLinkedList<Appointment>::iterator _nextAptIt;
    int _curID;
};

#endif // APPOINTMENTLIST_H
