#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include "model/appointment.h"
#include <QMap>
#include <QMutex>
#include <QObject>
#include <QLinkedList>
class Toaster;
class Calendar;

/**
  * Manages all active toaster notifications. Member functions in
  * this class should only be accessed from the GUI thread! This
  * class isn't thread-safe and needs to be able to spawn new Qt
  * dialogs.
  * \author Pieter De Decker
  */
class ToastManager : public QObject
{
    Q_OBJECT
public:
    ToastManager();
    ~ToastManager();

    /** Adds appointment list to the notification queue.
      * If no toaster is active, a new toaster will pop up. Otherwise
      * the list will be appended in the active toaster. */
    void addToQueue(Calendar* cal, const QString& title, const QList<Appointment>& list);
private slots:
    /** Catches a close signal from an active toaster and de-allocates it in
      * response to this signal. */
    void closeToaster(Toaster* toast);
private:
    /** Returns true if this function is being executed on the GUI thread. */
    bool isGUIThread();

    static const char* CLASSNAME;

    Toaster* _toaster;
};

#endif // TOASTMANAGER_H
