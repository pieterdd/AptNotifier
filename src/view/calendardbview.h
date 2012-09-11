#ifndef CALENDARDBVIEW_H
#define CALENDARDBVIEW_H

#include "model/calendar.h"
#include "model/appointment.h"
#include "view/toaster/toastmanager.h"
#include <QMap>
#include <QMenu>
#include <QMutex>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QSystemTrayIcon>
#include <QtGui/QMainWindow>
class CalendarDB;
class AptNotification;
class QListWidgetItem;

/**
  * Main application window. Also serves as the View counterpart to CalendarDB.
  * \author Pieter De Decker
  */
class CalendarDBView : public QMainWindow
{
    Q_OBJECT
public:
    CalendarDBView(CalendarDB* calDB, QWidget *parent = 0);
    ~CalendarDBView();
private:
    static const char* CLASSNAME;

    /** Configures controls and connections in the GUI. */
    void setupGUI();

    /** Updates the name tag of a certain calendar in the list widget. */
    void updateCalendarLabel(Calendar* cal);

    /** Updates the "all calendars online?" status of the application. */
    void refreshCalUpdateStatus();

    /** The associated CalendarDB in the Model. */
    CalendarDB* _calDB;
    QMutex _calsLock;

    // Controls
    QWidget _centralWidget;
    QVBoxLayout _mainLayout;
    QHBoxLayout _buttonContainer;
    QPushButton _btnAdd;
    QPushButton _btnRemove;
    QPushButton _btnHide;
    QListWidget _calList;
    QMap<Calendar*, QListWidgetItem*> _calItems;
    QMap<QListWidgetItem*, Calendar*> _widItems;
    QSystemTrayIcon _trayIcon;
    QMenu _trayMenu;
    bool _allCalsOnline;

    // Notification-related data
    ToastManager _tm;
private slots:
    /** Invokes calendar import dialog to add a new calendar. */
    void showNewCalendarDialog();

    /** Updates view to reflect successfully added calendar. */
    void registerCalendar(Calendar* cal);

    /** Updates view when the name of a calendar changes. */
    void processCalendarNameChange(Calendar*);

    /** Updates view when the status of a calendar changes. */
    void processCalendarStatusChange(Calendar* cal);

    /** Updates view when a calendar is removed. */
    void unregisterCalendar(Calendar* cal);

    /** Triggered when a calendar broadcasts newly ongoing events. */
    void processNewOngoingAptEvents(Calendar* cal, const QList<Appointment>& list);

    /** Triggered when a calendar broadcasts new reminders. */
    void processReminders(Calendar* cal, const QList<Appointment>& list);

    /** Shows an Invalid Calendar error for a given calendar. */
    void showInvalidCalendarFormatError(Calendar* cal);

    /** Updates the 'Remove' button state in response to a selection change. */
    void updateBtnRemoveState();

    /** Removes the calendars that are currently selected in the list. */
    void removeSelectedCalendars();

    /** Forces a global calendar update. */
    void updateCalendars();
};

#endif // CALENDARDBVIEW_H
