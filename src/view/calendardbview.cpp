#include "calendardbview.h"

#include "inputbox.h"
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
    _nfySpawnY = 0;
    _calDB = calDB;
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
    connect(_calDB, SIGNAL(newCalendarAdded(Calendar*)), this, SLOT(registerCalendar(Calendar*)));
    connect(_calDB, SIGNAL(removingCalendar(Calendar*)), this, SLOT(unregisterCalendar(Calendar*)));

    // Set up button group
    connect(&_btnAdd, SIGNAL(clicked()), this, SLOT(showNewCalendarDialog()));
    connect(&_btnRemove, SIGNAL(clicked()), this, SLOT(removeSelectedCalendars()));
    _btnAdd.setText("Add");
    _btnRemove.setEnabled(false);
    _btnRemove.setText("Remove");
    _buttonContainer.addWidget(&_btnAdd);
    _buttonContainer.addWidget(&_btnRemove);

    // Set up main layout
    _mainLayout.addWidget(&_calList);
    connect(&_calList, SIGNAL(itemSelectionChanged()), this, SLOT(updateBtnRemoveState()));
    _mainLayout.addLayout(&_buttonContainer);
    _centralWidget.setLayout(&_mainLayout);
    setCentralWidget(&_centralWidget);
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
    aptNfy->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    aptNfy->show();
}

void CalendarDBView::registerCalendar(Calendar* cal)
{
    connect(cal, SIGNAL(nameChanged(Calendar*)), this, SLOT(processCalendarNameChange(Calendar*)));
    connect(cal, SIGNAL(newOngoingAppointments(Calendar*,QLinkedList<Appointment>)), this, SLOT(processNewOngoingAptEvents(Calendar*,QLinkedList<Appointment>)));
    connect(cal, SIGNAL(newReminders(Calendar*,QLinkedList<Appointment>)), this, SLOT(processReminders(Calendar*,QLinkedList<Appointment>)));
    connect(cal, SIGNAL(calendarExceptionThrown(Calendar*,Calendar::ExceptionType)), this, SLOT(handleCalendarException(Calendar*,Calendar::ExceptionType)));

    // Create new widget item. The list widget takes ownership, so no need to delete.
    QListWidgetItem* newItem = new QListWidgetItem(QIcon(QPixmap::fromImage(cal->image()).scaledToHeight(_calList.height())), cal->name());
    _calList.addItem(newItem);
    _calItems[cal] = newItem;
    _widItems[newItem] = cal;
}

void CalendarDBView::processCalendarNameChange(Calendar* cal)
{
    QMap<Calendar*, QListWidgetItem*>::iterator it = _calItems.find(cal);
    assert(it != _calItems.end());
    it.value()->setText(cal->name());
}

void CalendarDBView::unregisterCalendar(Calendar* cal)
{
    // Search for the associated QListWidgetItem and associated iterators
    QMap<Calendar*, QListWidgetItem*>::iterator calIt = _calItems.find(cal);
    assert(calIt != _calItems.end());
    QListWidgetItem* widgetItem = *calIt;
    QMap<QListWidgetItem*, Calendar*>::iterator widIt = _widItems.find(widgetItem);
    assert(widIt != _widItems.end());

    // Remove it from the list widget
    _calList.removeItemWidget(widgetItem);
    delete widgetItem;      // TODO: should we do this? Check ownership in Qt docs.

    // Remove the calendar from the Calendar/QListWidgetItem dictionaries
    _calItems.erase(calIt);
    _widItems.erase(widIt);
}

void CalendarDBView::processNewOngoingAptEvents(Calendar *cal, const QLinkedList<Appointment> &list)
{
    createNotification(cal, "Ongoing appointments", list);
}

void CalendarDBView::processReminders(Calendar *cal, const QLinkedList<Appointment>& list)
{
    createNotification(cal, "Coming up", list);
}

void CalendarDBView::handleCalendarException(Calendar *cal, Calendar::ExceptionType type)
{
    if (type == Calendar::InvalidFormat)
        showInvalidCalendarFormatError(cal);
}

void CalendarDBView::showInvalidCalendarFormatError(Calendar*)
{
    QMessageBox msg;
    QString text = "A calendar you're trying to load could not be recognized. Please make sure that you have an active internet connection.\n\n";
    text += "You will NOT receive alerts from this calendar.";
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
        _calDB->addCalendar(newCalDialog.inputValue(), QColor(255, 0, 0));  // TODO: vary colors
    }
}

void CalendarDBView::notificationClosed(AptNotification* aptNfy)
{
    bool passedNfy = false;
    int lastKey = 0;

    // Reorder the notification stack accordingly
    _nfyStackLock.lock();
    QMap<int, AptNotification*>::iterator it = _nfyStack.begin();

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
