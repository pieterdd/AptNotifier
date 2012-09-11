#include "aptbundle.h"

#include "model/calendar.h"

AptBundle::AptBundle(Calendar* cal, const QString& title, const QList<Appointment>& list) {
    _cal = cal;
    _title = title;
    _list = list;
}
