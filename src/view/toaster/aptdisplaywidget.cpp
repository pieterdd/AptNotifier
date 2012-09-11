#include "aptdisplaywidget.h"

#include "model/calendar.h"
#include "model/appointment.h"

AptDisplayWidget::AptDisplayWidget(QWidget* parent) :
    QWidget(parent) {
    setupGUI();
}

void AptDisplayWidget::loadAppointment(Calendar* cal, const Appointment& apt)
{
    // Appointment info
    QString timeStr = apt.timeString();
    _lblAppointment.setText("<b style='font-size: 16px; font-family: Arial'>" + apt.summary() + "</b><br />" + timeStr);

    // Calendar info
    QImage calImg = cal->image().scaledToHeight(14);
    Calendar::drawBorder(calImg, 1, QColor(25, 25, 25));
    _lblCalImg.setPixmap(QPixmap::fromImage(calImg));
    _lblCalName.setText(cal->name());
}

void AptDisplayWidget::setupGUI() {
    // Calendar layout
    _hlCal.addWidget(&_lblCalImg);
    _hlCal.addWidget(&_lblCalName);
    _hlCal.addSpacerItem(new QSpacerItem(1, 0, QSizePolicy::Expanding));

    // Main layout
    setLayout(&_vlMain);
    _vlMain.addWidget(&_lblAppointment);
    _vlMain.addLayout(&_hlCal);
    _vlMain.addSpacerItem(new QSpacerItem(0, 1, QSizePolicy::Expanding));
}
