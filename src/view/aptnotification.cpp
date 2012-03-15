#include "aptnotification.h"

#include "model/calendar.h"
#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

const int AptNotification::WIDTH = 300;
const int AptNotification::HEIGHT = 100;
const int AptNotification::BORDERSPACING = 10;

AptNotification::AptNotification(Calendar *cal, const QString &title, const QLinkedList<Appointment> &aptList, unsigned stackPos)
{
    _cal = cal;
    _lblNfyTitle.setText("<b>" + cal->name() + "</b> -- " + title);
    _aptList = aptList;

    setStackPosition(stackPos);
    setFixedSize(WIDTH, HEIGHT);
    setupGUI();

    // Activate notification cycling
    _nextAptIt = _aptList.begin();
    nextAppointment();
    connect(&_nfyTimer, SIGNAL(timeout()), this, SLOT(nextAppointment()));
    _nfyTimer.setSingleShot(true);
    _nfyTimer.start(5000);
}

AptNotification::~AptNotification()
{
}

void AptNotification::setStackPosition(unsigned stackPos)
{
    QDesktopWidget* qdw = QApplication::desktop();

    // Get the available screen real estate (minus taskbar) and
    // position the window appropriately
    setGeometry(qdw->availableGeometry().width() - WIDTH,
                qdw->availableGeometry().height() - (HEIGHT)*(stackPos + 1),
                WIDTH, HEIGHT);
}

void AptNotification::setupGUI()
{
    // TODO: add visual representation of cycle timer and calendar icon
    setWindowFlags(Qt::Popup);
    connect(this, SIGNAL(finished(int)), this, SLOT(sendNotificationFinished()));
    connect(&_btnNfyClose, SIGNAL(clicked()), this, SLOT(sendNotificationFinished()));
    setContentsMargins(0, 0, 0, 0);
    setLayout(&_mainLayout);

    // Main > Top
    _frmTop.setStyleSheet("background-color: #ccc");
    _frmTop.setFrameShadow(QFrame::Raised);
    _lblCalImage.setPixmap(QPixmap::fromImage(_cal->image()).scaledToHeight(16));
    _btnNfyClose.setText("X");
    _btnNfyClose.setFixedSize(20, 20);
    QHBoxLayout* frameLayout = new QHBoxLayout();
    frameLayout->addWidget(&_lblCalImage);
    frameLayout->addWidget(&_lblNfyTitle, 1);
    frameLayout->addWidget(&_btnNfyClose);
    _frmTop.setLayout(frameLayout);

    // Main > Body > Left
    _bodyLeftLayout.setContentsMargins(0, 0, 0, 0);
    _bodyLeftLayout.addWidget(&_lblAptStart);

    // Main > Body
    _bodyLayout.setContentsMargins(10, 10, 10, 10);
    _bodyLayout.addLayout(&_bodyLeftLayout);
    _lblAptTitle.setStyleSheet("font-weight: bold");
    _bodyLayout.addWidget(&_lblAptTitle, 1);

    // Main layout
    _mainLayout.setContentsMargins(0, 0, 0, 0);
    _mainLayout.addWidget(&_frmTop, 0);
    _mainLayout.addLayout(&_bodyLayout, 1);
}

void AptNotification::sendNotificationFinished()
{
    emit notificationClosed(this);
}

void AptNotification::nextAppointment()
{
    // As long as we have appointments, we can advance to the next one.
    if (_nextAptIt != _aptList.end()) {
        const Appointment& apt = *_nextAptIt;
        QString start = Appointment::composeShortDateTime(apt.start());
        _lblAptStart.setText(start);
        _lblAptTitle.setText(apt.summary());
        ++_nextAptIt;
        _nfyTimer.start();
    }
    // If we're out of appointments, close the window.
    else {
        sendNotificationFinished();
    }
}
