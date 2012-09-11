#ifndef APTDISPLAYWIDGET_H
#define APTDISPLAYWIDGET_H

#include "model/appointment.h"
#include <QLabel>
#include <QWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLinkedList>
class Calendar;

class AptDisplayWidget : public QWidget
{
    Q_OBJECT
public:
    AptDisplayWidget(QWidget* parent = 0);
    
    /** Displays an appointment from a certain calendar in this widget. */
    void loadAppointment(Calendar* cal, const Appointment& apt);
private:
    /** Hooks up all widgets. */
    void setupGUI();
    
    // Layout
    QVBoxLayout _vlMain;
    QHBoxLayout _hlCal;
    QHBoxLayout _hlBottom;

    // Controls
    QLabel _lblAppointment;
    QLabel _lblCalImg;
    QLabel _lblCalName;
};

#endif // APTDISPLAYWIDGET_H
