#include "calendardbview.h"

#include "inputbox.h"
#include "stringres.h"
#include "aptnotification.h"
#include "model/calendardb.h"
#include "model/appointment.h"
#include <cassert>
#include <QMetaType>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QtConcurrentRun>

CalendarDBView::CalendarDBView(CalendarDB *calDB, QWidget *parent)
    : QMainWindow(parent)
{
    assert(_calDB);
    _nfySpawnY = 0;
    _calDB = calDB;
    _allCalsOnline = true;
    setupGUI();

    // Load the calendar list from a file
    _calDB->loadCalendars();
}

CalendarDBView::~CalendarDBView()
{
    // Close all open notifications. They will be deallocated by a signal
    // that gets triggered when the notification is closed.
    QMap<int, AptNotification*>::iterator begin;
    do {
        _nfyStackLock.lock();
        begin = _nfyStack.begin();
        if (begin != _nfyStack.end())
            begin.value()->close();
        _nfyStackLock.unlock();
    } while (begin != _nfyStack.end());
}

void CalendarDBView::setupGUI()
{
    assert(_calDB);
    connect(_calDB, SIGNAL(newCalendarAdded(Calendar*)), this, SLOT(registerCalendar(Calendar*)));
    connect(_calDB, SIGNAL(removingCalendar(Calendar*)), this, SLOT(unregisterCalendar(Calendar*)));
    setWindowTitle(StringRes::appName() + " " + StringRes::appVersion());

    // Set up button group
    connect(&_btnAdd, SIGNAL(clicked()), this, SLOT(showNewCalendarDialog()));
    connect(&_btnRemove, SIGNAL(clicked()), this, SLOT(removeSelectedCalendars()));
    connect(&_btnHide, SIGNAL(clicked()), this, SLOT(hide()));
    _btnAdd.setText("Add");
    _btnRemove.setEnabled(false);
    _btnRemove.setText("Remove");
    _btnHide.setText("Hide window");
    _buttonContainer.addWidget(&_btnAdd);
    _buttonContainer.addWidget(&_btnRemove);
    _buttonContainer.addWidget(&_btnHide);

    // Set up main layout
    _mainLayout.addWidget(&_calList);
    connect(&_calList, SIGNAL(itemSelectionChanged()), this, SLOT(updateBtnRemoveState()));
    _mainLayout.addLayout(&_buttonContainer);
    _centralWidget.setLayout(&_mainLayout);
    setCentralWidget(&_centralWidget);

    // Tray icon
    _trayIcon.setIcon(QIcon(":/general/appointment.png"));
    _trayIcon.setToolTip(StringRes::appName());
    _trayIcon.show();
    QAction* showWindow = _trayMenu.addAction("Show window");
    QAction* updateAll = _trayMenu.addAction("Update all");
    QAction* quitAct = _trayMenu.addAction("Quit");
    connect(showWindow, SIGNAL(triggered()), this, SLOT(show()));
    connect(updateAll, SIGNAL(triggered()), this, SLOT(updateCalendars()));
    connect(quitAct, SIGNAL(triggered()), this, SLOT(close()));
    _trayIcon.setContextMenu(&_trayMenu);
}

void CalendarDBView::createNotification(Calendar* cal, const QString &title, const QLinkedList<Appointment> &list)
{
    _nfyStackLock.lock();

    // Add a new notification window to the stack
    AptNotification* aptNfy = new AptNotification(cal, title, list, _nfyStack.size());
    connect(aptNfy, SIGNAL(notificationClosed(AptNotification*)), this, SLOT(notificationClosed(AptNotification*)));
    _nfyStack[_nfyStack.size()] = aptNfy;

    _nfyStackLock.unlock();

    // Activate the window
    aptNfy->show();
}

void CalendarDBView::updateCalendarLabel(Calendar *cal)
{
    QString statusName;
    switch (cal->status()) {
    default:
    case Calendar::NotLoaded:
        statusName = "not loaded";
        break;
    case Calendar::Online:
        statusName = "online";
        break;
    case Calendar::Offline:
        statusName = "offline";
        break;
    }

    QMap<Calendar*, QListWidgetItem*>::iterator it = _calItems.find(cal);
    assert(it != _calItems.end());
    it.value()->setText(cal->name() + " (" + statusName + ")");
}

void CalendarDBView::refreshCalUpdateStatus()
{
    _calsLock.lock();

    // Are all known calendars now online?
    _allCalsOnline = true;
    for (QMap<Calendar*, QListWidgetItem*>::iterator it = _calItems.begin();
         it != _calItems.end() && _allCalsOnline; ++it) {
        Calendar* curCal = it.key();
        if (curCal->status() != Calendar::Online) {
            _allCalsOnline = false;
            setWindowIcon(QIcon(":/general/appointmentgrey.png"));
            _trayIcon.setToolTip(StringRes::appName() + " -- Some calendars are offline.");
            _trayIcon.setIcon(QIcon(":/general/appointmentgrey.png"));
        }
    }

    // All found calendars are online, we're good to go.
    if (_allCalsOnline) {
        setWindowIcon(QIcon(":/general/appointment.png"));
        _trayIcon.setToolTip(StringRes::appName());
        _trayIcon.setIcon(QIcon(":/general/appointment.png"));
    }

    _calsLock.unlock();
}

void CalendarDBView::registerCalendar(Calendar* cal)
{
    connect(cal, SIGNAL(nameChanged(Calendar*)), this, SLOT(processCalendarNameChange(Calendar*)));
    connect(cal, SIGNAL(newOngoingAppointments(Calendar*,QLinkedList<Appointment>)), this, SLOT(processNewOngoingAptEvents(Calendar*,QLinkedList<Appointment>)));
    connect(cal, SIGNAL(newReminders(Calendar*,QLinkedList<Appointment>)), this, SLOT(processReminders(Calendar*,QLinkedList<Appointment>)));
    connect(cal, SIGNAL(formatNotRecognized(Calendar*)), this, SLOT(showInvalidCalendarFormatError(Calendar*)));
    connect(cal, SIGNAL(statusChanged(Calendar*)), this, SLOT(processCalendarStatusChange(Calendar*)));

    // Create new widget item. The list widget takes ownership, so no need to delete.
    _calsLock.lock();
    QListWidgetItem* newItem = new QListWidgetItem(QIcon(QPixmap::fromImage(cal->image()).scaledToHeight(_calList.height())), cal->name());
    _calList.addItem(newItem);
    _calItems[cal] = newItem;
    _widItems[newItem] = cal;
    _calsLock.unlock();

    // Refresh the global calendar update status.
    refreshCalUpdateStatus();
}

void CalendarDBView::processCalendarNameChange(Calendar* cal)
{
    // TODO VERIFY -- Thread safety: this slot will not be executing for two
    // objects simultaneously.

    // Only an update of the list widget is needed
    updateCalendarLabel(cal);
}

void CalendarDBView::processCalendarStatusChange(Calendar *cal)
{
    // TODO VERIFY -- Thread safety: this slot will not be executing for two
    // objects simultaneously.

    // Update the list widget
    updateCalendarLabel(cal);

    // Refresh the global calendar update status
    refreshCalUpdateStatus();
}

void CalendarDBView::unregisterCalendar(Calendar* cal)
{
    _calsLock.lock();

    // Search for the associated QListWidgetItem and associated iterators
    QMap<Calendar*, QListWidgetItem*>::iterator calIt = _calItems.find(cal);
    assert(calIt != _calItems.end());
    QListWidgetItem* widgetItem = *calIt;
    QMap<QListWidgetItem*, Calendar*>::iterator widIt = _widItems.find(widgetItem);
    assert(widIt != _widItems.end());

    // Remove it from the list widget
    _calList.removeItemWidget(widgetItem);
    delete widgetItem;      // Deletion needed because _calList lost ownership

    // Remove the calendar from the Calendar/QListWidgetItem dictionaries
    _calItems.erase(calIt);
    _widItems.erase(widIt);

    _calsLock.unlock();

    // Refresh the global calendar update status.
    refreshCalUpdateStatus();
}

void CalendarDBView::processNewOngoingAptEvents(Calendar *cal, const QLinkedList<Appointment> &list)
{
    createNotification(cal, "Now in progress", list);
}

void CalendarDBView::processReminders(Calendar *cal, const QLinkedList<Appointment>& list)
{
    createNotification(cal, "Event reminder", list);
}

void CalendarDBView::showInvalidCalendarFormatError(Calendar* cal)
{
    QMessageBox msg;
    QString text = "One of your calendars could not be recognized. Please make sure that you have an active internet connection and verify the address of the calendar.\n\n";
    text += "You will NOT receive alerts from this calendar.\n";
    text += "URL: " + cal->url();
    msg.setText(text);
    msg.setIcon(QMessageBox::Warning);
    msg.exec();
}

void CalendarDBView::showNewCalendarDialog()
{
    InputBox newCalDialog("Enter the URL of the calendar below.", QRegExp("^(http|https)://[a-z0-9]+([-.]{1}[a-z0-9]+)*.[a-z]{2,5}(([0-9]{1,5})?/?.*)$"), this);
    int returnCode = newCalDialog.exec();

    // Try to add the calendar if the input is accepted
    if (returnCode == QDialog::Accepted) {
        _calDB->addCalendar(newCalDialog.inputValue(), _calDB->composeNextColor());
    }
}

void CalendarDBView::notificationClosed(AptNotification* aptNfy)
{
    bool passedNfy = false;
    int lastKey = 0;

    // Reorder the notification stack accordingly
    _nfyStackLock.lock();
    QMap<int, AptNotification*>::iterator it = _nfyStack.begin();

    // TODO: stacking produces unexpected results in
    // a test with 3 calendars generating notifications
    // simultaneously
    while (it != _nfyStack.end()) {
        AptNotification* curNfy = it.value();

        // Found notification window?
        if (curNfy == aptNfy) {
            passedNfy = true;
            aptNfy->deleteLater();
            lastKey = it.key();
            it = _nfyStack.erase(it);
        }
        // All subsequent windows need to be moved to the front
        else if (passedNfy) {
            assert(!_nfyStack.contains(lastKey));
            _nfyStack[lastKey] = curNfy;
            lastKey = it.key();
            it = _nfyStack.erase(it);
            curNfy->setStackPosition(lastKey);
        } else
            ++it;
    }

    _nfyStackLock.unlock();
}

void CalendarDBView::updateBtnRemoveState()
{
    // 'Remove' button can be used if exactly one
    // calendar is selected.
    if (_calList.selectedItems().count() == 1)
        _btnRemove.setEnabled(true);
    else
        _btnRemove.setEnabled(false);
}

void CalendarDBView::removeSelectedCalendars()
{
    QList<QListWidgetItem*> select = _calList.selectedItems();

    // Issue a delete request for all selected items
    for (QList<QListWidgetItem*>::iterator it = select.begin(); it != select.end(); ++it) {
        QListWidgetItem* widgetItem = *it;

        assert(_widItems.contains(widgetItem));
        Calendar* cal = _widItems[widgetItem];
        _calDB->removeCalendar(cal);
    }
}

void CalendarDBView::updateCalendars()
{
    _trayIcon.showMessage(StringRes::appName(), "Hang on while we refresh your calendars...");
    assert(_calDB);
    _calDB->updateCalendars();
}
