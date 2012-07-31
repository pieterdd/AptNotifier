#include "appointmentlist.h"

AppointmentList::AppointmentList(Calendar* cal, const QLinkedList<Appointment>& aptList) {
    _aptList = aptList;
    setupGUI(cal);
}

void AppointmentList::start() {
    // Activate notification cycling
    _nextAptIt = _aptList.begin();
    nextSlide();
    connect(&_nfyTimer, SIGNAL(timeout()), this, SLOT(nextSlide()));
    _nfyTimer.setSingleShot(true);
    _nfyTimer.start(7000);
}

void AppointmentList::nextSlide() {
    // As long as we have appointments, we can advance to the next one.
    if (_nextAptIt != _aptList.end()) {
        const Appointment& apt = *_nextAptIt;
        QString start = Appointment::composeShortDateTime(apt.start());
        _lblAppointment.setText(start + " <b>" + apt.summary() + "</b>");
        ++_nextAptIt;
        _nfyTimer.start();

        // If this item was the last in line, disable the next button
        if (_nextAptIt == _aptList.end())
            _btnNext.setEnabled(false);
    }
    // If we're out of appointments, we're done.
    else {
        emit doneShowingInfo();
    }
}

void AppointmentList::setupGUI(Calendar* cal) {
    // Calendar layout
    QImage calImg = cal->image().scaledToHeight(14);
    Calendar::drawBorder(calImg, 1, QColor(25, 25, 25));
    _lblCalImg.setPixmap(QPixmap::fromImage(calImg));
    _lblCalName.setText(cal->name());
    _hlCal.addWidget(&_lblCalImg);
    _hlCal.addWidget(&_lblCalName);
    _hlCal.addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));

    // Bottom layout
    _btnNext.setText(">");
    connect(&_btnNext, SIGNAL(clicked()), this, SLOT(nextSlide()));
    _hlBottom.addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));
    _hlBottom.addWidget(&_btnNext);

    // Main layout
    setLayout(&_vlMain);
    _vlMain.addWidget(&_lblAppointment);
    _vlMain.addLayout(&_hlCal);
    _vlMain.addSpacerItem(new QSpacerItem(0, 1, QSizePolicy::Expanding));
    _vlMain.addLayout(&_hlBottom);
}
