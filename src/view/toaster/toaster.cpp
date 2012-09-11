#include "toaster.h"

#include "view/toaster/infowidget.h"
#include <QFont>
#include <cassert>
#include <QPainter>
#include <QSpacerItem>

Toaster::Toaster()
    : QDialog(NULL)
{
    _curAptID = 0;
    _curBundle = 0;
    _curApt = -1;
    setupGUI();
}

Toaster::~Toaster() {
}

void Toaster::appendBundle(const AptBundle& bundle) {
    assert(bundle.list().size() > 0);
    _bundles.append(bundle);
    _lblCounter.setText(QString::number(_curAptID) + "/" + QString::number(totalApts()));
}

void Toaster::show() {
    QDialog::show();
    nextSlide();
}

void Toaster::setupGUI() {
    // Window setup
    setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(WIDTH, HEIGHT);

    // Bottom layout
    _btnBack.setText("<");
    _btnBack.setMaximumWidth(30);
    connect(&_btnBack, SIGNAL(clicked()), this, SLOT(prevSlide()));
    _btnNext.setText(">");
    _btnNext.setMaximumWidth(30);
    connect(&_btnNext, SIGNAL(clicked()), this, SLOT(nextSlide()));
    _hlBottom.setContentsMargins(5, 0, 5, 5);
    _hlBottom.addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));
    _hlBottom.addWidget(&_btnBack, 0, Qt::AlignLeft);
    _hlBottom.addWidget(&_lblCounter, 0, Qt::AlignCenter);
    _hlBottom.addWidget(&_btnNext, 0, Qt::AlignRight);

    // Rest of the layouts and widgets
    _vlMain.setContentsMargins(0, 0, 0, 0);
    _lblTitle.setContentsMargins(15, 0, 0, 0);
    _lblTitle.setStyleSheet("color: #fff; font-family: Arial; font-size: 16px; font-weight: bold");
    _lblTitle.setFixedHeight(35);
    _vlMain.addWidget(&_lblTitle);
    _vlMain.addLayout(&_hlBody);
    _vlMain.addSpacerItem(new QSpacerItem(0, 5));
    _hlBody.addSpacerItem(new QSpacerItem(100, 0));
    _vlInfo.addWidget(&_adg, 1);
    _vlInfo.addLayout(&_hlBottom);
    _hlBody.addLayout(&_vlInfo, 2);
    _hlBody.addSpacerItem(new QSpacerItem(1, 0));
    setLayout(&_vlMain);

    // Timer setup
    connect(&_slideTimer, SIGNAL(timeout()), this, SLOT(nextSlide()));
    _slideTimer.setInterval(5000);
}

int Toaster::totalApts() const {
    int total = 0;
    foreach (AptBundle bundle, _bundles) {
        total += bundle.list().size();
    }

    return total;
}

void Toaster::loadAppointment() {
    assert(_curBundle < _bundles.size());
    AptBundle aptBundle = _bundles.at(_curBundle);
    assert(_curApt < aptBundle.list().size());
    Appointment apt = aptBundle.list().at(_curApt);

    // Set the new title and pass the appointment to the display widget
    _lblTitle.setText(aptBundle.title());
    _lblCounter.setText(QString::number(_curAptID) + "/" + QString::number(totalApts()));
    _adg.loadAppointment(aptBundle.cal(), apt);
}

void Toaster::nextSlide() {
    _slideTimer.stop();

    // Send close request if no next slide is available
    if (_curAptID >= totalApts()) {
        emit closeRequested(this);
        return;
    }

    ++_curApt;
    assert(_curBundle < _bundles.size());
    AptBundle aptBundle = _bundles.at(_curBundle);
    if (_curApt >= aptBundle.list().size()) {
        // Jump to the next bundle
        ++_curBundle;
        _curApt = 0;
    }

    ++_curAptID;
    loadAppointment();
    _slideTimer.start();
}

void Toaster::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.drawImage(0, 0, QImage(":/bg/toaster.png"));
}
