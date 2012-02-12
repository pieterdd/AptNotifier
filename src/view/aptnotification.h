/**
  * \class AptNotification
  * \author Pieter De Decker
  *
  * \brief Toaster-like notification window that displays appointment information.
  */
#ifndef APTNOTIFICATION_H
#define APTNOTIFICATION_H

#include <QDialog>

#include "model/appointment.h"
#include <QLabel>
#include <QFrame>
#include <QTimer>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLinkedList>
class Calendar;

class AptNotification : public QDialog
{
    Q_OBJECT
public:
    AptNotification(Calendar* cal, const QString& title, const QLinkedList<Appointment>& aptList, unsigned stackPos);
    ~AptNotification();

    /** Using the position of this notification in the stack, the window position is updated. */
    void setStackPosition(unsigned stackPos);

    // Window geometry constants
    static const int WIDTH;
    static const int HEIGHT;
    static const int BORDERSPACING;
private:
    /** Constructs GUI and wires controls. */
    void setupGUI();

    // Data
    Calendar* _cal;
    QLinkedList<Appointment> _aptList;

    // Controls and layouts
    QVBoxLayout _mainLayout;
    QFrame _frmTop;
    QLabel _lblNfyTitle;
    QPushButton _btnNfyClose;
    QHBoxLayout _bodyLayout;
    QVBoxLayout _bodyLeftLayout;
    QLabel _lblCalImage;
    QLabel _lblAptStart;
    QLabel _lblAptTitle;

    /** Notification cycle timer */
    QTimer _nfyTimer;
    QLinkedList<Appointment>::iterator _nextAptIt;
private slots:
    /** Helper that lets us send a "dialog closed" signal with the
      * dialog instance attached to it.
      */
    void sendDialogCloseEvent();

    /** Hops to the next appointment in the list. */
    void nextAppointment();
signals:
    /** Broadcast when the notification has closed. */
    void notificationClosed(AptNotification*);
};

#endif // APTNOTIFICATION_H
