/**
 * CalendarDBView
 * \author Pieter De Decker
 *
 * \brief Main application window. Also serves as the View counterpart to CalendarDB.
 */
#ifndef CALENDARDBVIEW_H
#define CALENDARDBVIEW_H

#include "model/calendar.h"
#include "model/appointment.h"
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

class CalendarDBView : public QMainWindow
{
    Q_OBJECT
public:
    CalendarDBView(CalendarDB* calDB, QWidget *parent = 0);
    ~CalendarDBView();
private:
    /** Configures controls and connections in the GUI. */
    void setupGUI();

    /** Creates a notification with a certain title and the appointments in the supplied list. */
    void createNotification(Calendar* cal, const QString& title, const QLinkedList<Appointment>& list);

    /** Shows an Invalid Calendar error for a given calendar. */
    void showInvalidCalendarFormatError(Calendar*);

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

    // Notification-related data
    QMutex _nfyStackLock;
    QMap<int, AptNotification*> _nfyStack;
    int _nfySpawnY;
private slots:
    /** Invokes calendar import dialog to add a new calendar. */
    void showNewCalendarDialog();

    /** Updates view to reflect successfully added calendar. */
    void registerCalendar(Calendar* cal);

    /** Updates view when the name of a calendar changes. */
    void processCalendarNameChange(Calendar*);

    /** Updates view when a calendar is removed. */
    void unregisterCalendar(Calendar* cal);

    /** Triggered when a calendar broadcasts newly ongoing events. */
    void processNewOngoingAptEvents(Calendar* cal, const QLinkedList<Appointment>& list);

    /** Triggered when a calendar broadcasts new reminders. */
    void processReminders(Calendar* cal, const QLinkedList<Appointment>& list);

    /** If a calendar broadcasts an exception, this function handles it.
      * We've used this system as opposed to C++ exception handling because
      * this form of exception handling can easily be implemented asynchronously. */
    void handleCalendarException(Calendar* cal, Calendar::ExceptionType type);

    /** When a notification dialog closes, we need to react to this
      * to make the notification stacking mechanism work. */
    void notificationClosed(AptNotification* aptNfy);

    /** Updates the 'Remove' button state in response to a selection change. */
    void updateBtnRemoveState();

    /** Removes the calendars that are currently selected in the list. */
    void removeSelectedCalendars();
};

#endif // CALENDARDBVIEW_H
