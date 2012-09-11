#include "toastmanager.h"

#include "aptbundle.h"
#include "model/logger.h"
#include "model/calendar.h"
#include "view/toaster/toaster.h"
#include <cassert>
#include <QThread>
#include <QApplication>
#include <QDesktopWidget>

const char* ToastManager::CLASSNAME = "ToastManager";

ToastManager::ToastManager() {
    _toaster = NULL;
}

ToastManager::~ToastManager() {
    delete _toaster;
    _toaster = NULL;
}

void ToastManager::addToQueue(Calendar *cal, const QString &title, const QList<Appointment>& list) {
    assert(isGUIThread());

    // Spawn new toaster
    if (!_toaster) {
        Logger::instance()->add(CLASSNAME, "Creating new toaster with content '" + title + "' for calendar " + cal->name() + "...");
        _toaster = new Toaster();
        connect(_toaster, SIGNAL(closeRequested(Toaster*)), this, SLOT(closeToaster(Toaster*)));
        QDesktopWidget* qdw = QApplication::desktop();
        _toaster->setGeometry(qdw->availableGeometry().width() - Toaster::WIDTH,
                    qdw->availableGeometry().height() - Toaster::HEIGHT,
                    Toaster::WIDTH, Toaster::HEIGHT);
        _toaster->appendBundle(AptBundle(cal, title, list));
        _toaster->show();
    }
    // Add notifications to existing toaster
    else {
        Logger::instance()->add(CLASSNAME, "Adding toaster content '" + title + "' for calendar " + cal->name() + "...");
        _toaster->appendBundle(AptBundle(cal, title, list));
    }
}

void ToastManager::closeToaster(Toaster* toast) {
    assert(toast == _toaster);
    delete _toaster;
    _toaster = NULL;
}

bool ToastManager::isGUIThread() {
    return QCoreApplication::instance() && QThread::currentThread() == QCoreApplication::instance()->thread();
}


