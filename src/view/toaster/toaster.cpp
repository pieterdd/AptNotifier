#include "toaster.h"

#include "model/logger.h"
#include "view/toaster/infowidget.h"
#include <QFont>
#include <cassert>
#include <QPainter>
#include <QSpacerItem>

const char* Toaster::CLASSNAME = "Toaster";

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
    _btnPrev.setText("<");
    _btnPrev.setMaximumWidth(30);
    connect(&_btnPrev, SIGNAL(clicked()), this, SLOT(prevSlide()));
    _btnNext.setText(">");
    _btnNext.setMaximumWidth(30);
    connect(&_btnNext, SIGNAL(clicked()), this, SLOT(nextSlide()));
    _hlBottom.setContentsMargins(0, 0, 10, 5);
    _hlBottom.addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));
    _hlBottom.addWidget(&_btnPrev, 0, Qt::AlignLeft);
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
    Logger::instance()->add(CLASSNAME, this, "Loading slide (" + QString::number(_curBundle) + ", " + QString::number(_curApt) + ")...");

    assert(exists(_curBundle, _curApt));
    AptBundle aptBundle = _bundles.at(_curBundle);
    Appointment apt = aptBundle.list().at(_curApt);

    // Set the new title and pass the appointment to the display widget
    _lblTitle.setText(aptBundle.title());
    _lblCounter.setText(QString::number(_curAptID) + "/" + QString::number(totalApts()));
    _adg.loadAppointment(aptBundle.cal(), apt);

    // Enable previous button?
    if (exists(_curBundle - 1, 0) || exists(_curBundle, _curApt - 1))
        _btnPrev.setEnabled(true);
    else
        _btnPrev.setEnabled(false);

    // Enable next button?
    if (exists(_curBundle + 1, 0) || exists(_curBundle, _curApt + 1))
        _btnNext.setEnabled(true);
    else
        _btnNext.setEnabled(false);
}

bool Toaster::exists(int bundleID, int aptID) {
    if (bundleID >= _bundles.size() || bundleID < 0 || aptID < 0)
        return false;
    const AptBundle& bundle = _bundles.at(bundleID);
    return aptID < bundle.list().size();
}

void Toaster::prevSlide() {
    Logger::instance()->add(CLASSNAME, this, "Loading previous slide...");

    if (_curAptID <= 0) {
        Logger::instance()->add(CLASSNAME, this, "Can't go back any further");
        return;
    }
    _slideTimer.stop();

    // Try to go to the previous appointment in this bundle
    if (_curApt > 0) {
        --_curApt;
        assert(exists(_curBundle, _curApt));
    }
    // Or else go to the last appointment in the previous bundle
    else {
        assert(_curBundle > 0);
        --_curBundle;
        _curApt = _bundles.at(_curBundle).list().size() - 1;
        assert(exists(_curBundle, _curApt));
    }

    --_curAptID;
    loadAppointment();
    _slideTimer.start();
}

void Toaster::nextSlide() {
    Logger::instance()->add(CLASSNAME, this, "Loading next slide...");
    _slideTimer.stop();

    // Send close request if no next slide is available
    if (_curAptID >= totalApts()) {
        Logger::instance()->add(CLASSNAME, this, "Finished showing slides");
        emit closeRequested(this);
        return;
    }

    // Try to go to the next appointment in this bundle
    if (exists(_curBundle, _curApt + 1)) {
        ++_curApt;
    }
    // Or else go to the first appointment in the next bundle
    else {
        ++_curBundle;
        _curApt = 0;
        assert(exists(_curBundle, _curApt));
    }

    ++_curAptID;
    loadAppointment();
    _slideTimer.start();
}

void Toaster::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.drawImage(0, 0, QImage(":/bg/toaster.png"));
}
