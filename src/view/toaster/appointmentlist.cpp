#include "appointmentlist.h"

AppointmentList::AppointmentList(Calendar* cal, const QLinkedList<Appointment>& aptList) {
    // TODO: _cal isn't used
    _cal = cal;
    _aptList = aptList;
    setupGUI();
}

void AppointmentList::start() {
    // Activate notification cycling
    _nextAptIt = _aptList.begin();
    nextSlide();
    connect(&_nfyTimer, SIGNAL(timeout()), this, SLOT(nextSlide()));
    _nfyTimer.setSingleShot(true);
    _nfyTimer.start(5000);
}

void AppointmentList::nextSlide() {
    // As long as we have appointments, we can advance to the next one.
    if (_nextAptIt != _aptList.end()) {
        const Appointment& apt = *_nextAptIt;
        QString start = Appointment::composeShortDateTime(apt.start());
        _lblAppointment.setText(start + " " + apt.summary());
        _lblCalName.setText(_cal->name());
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

void AppointmentList::setupGUI() {
    setLayout(&_vlMain);
    _vlMain.addWidget(&_lblAppointment);
    _vlMain.addWidget(&_lblCalName);
    _btnNext.setText(">");
    connect(&_btnNext, SIGNAL(clicked()), this, SLOT(nextSlide()));
    _hlBottom.addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));
    _hlBottom.addWidget(&_btnNext);
    _vlMain.addLayout(&_hlBottom);
}
