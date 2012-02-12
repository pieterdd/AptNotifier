#include "aptnotification.h"

#include <QVBoxLayout>
#include <QApplication>
#include <QDesktopWidget>

const int AptNotification::WIDTH = 250;
const int AptNotification::HEIGHT = 100;
const int AptNotification::BORDERSPACING = 10;

AptNotification::AptNotification(Calendar *cal, const QString &title, const QLinkedList<Appointment> &aptList, unsigned stackPos)
{
    _cal = cal;
    _lblNfyTitle.setText(title);
    _aptList = aptList;

    _lblAptStart = new QLabel(this);
    _lblAptTitle = new QLabel(this);

    setStackPosition(stackPos);
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
    delete _lblAptStart;
    delete _lblAptTitle;
}

void AptNotification::setStackPosition(unsigned stackPos)
{
    QDesktopWidget* qdw = QApplication::desktop();
    setGeometry(qdw->screen()->width() - (WIDTH - BORDERSPACING)*(stackPos + 1),
                qdw->screen()->height() - (HEIGHT - BORDERSPACING)*(stackPos + 1),
                WIDTH, HEIGHT);
}

void AptNotification::setupGUI()
{
    // TODO: add visual representation of cycle timer and calendar icon
    connect(this, SIGNAL(finished(int)), this, SLOT(sendDialogCloseEvent()));
    setContentsMargins(0, 0, 0, 0);
    setLayout(&_mainLayout);

    // Top frame
    _frmTop.setStyleSheet("background-color: #ccc");
    _frmTop.setFrameShadow(QFrame::Raised);
    _btnNfyClose.setText("X");
    _btnNfyClose.setFixedSize(20, 20);
    QHBoxLayout* frameLayout = new QHBoxLayout();
    frameLayout->addWidget(&_lblNfyTitle);
    frameLayout->addWidget(&_btnNfyClose);
    _frmTop.setLayout(frameLayout);

    // Body section
    _bodyLayout.setContentsMargins(10, 0, 10, 0);
    _bodyLayout.addWidget(_lblAptStart);
    _bodyLayout.addWidget(_lblAptTitle);

    // Finalize layout
    _mainLayout.setContentsMargins(0, 0, 0, 0);
    _mainLayout.addWidget(&_frmTop, 0);
    _mainLayout.addLayout(&_bodyLayout, 1);
}

void AptNotification::sendDialogCloseEvent()
{
    emit notificationClosed(this);
}

void AptNotification::nextAppointment()
{
    // As long as we have appointments, we can advance to the next one.
    if (_nextAptIt != _aptList.end()) {
        const Appointment& apt = *_nextAptIt;
        QString start = Appointment::composeShortDateTime(apt.start());
        _lblAptStart->setText(start);
        _lblAptTitle->setText(apt.summary());
        ++_nextAptIt;
        _nfyTimer.start();
    }
    // If we're out of appointments, close the window.
    else {
        close();
    }
}
