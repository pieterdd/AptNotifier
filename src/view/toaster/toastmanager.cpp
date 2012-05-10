#include "toastmanager.h"

#include "view/toaster/toaster.h"
#include <cassert>
#include <QApplication>
#include <QDesktopWidget>

ToastManager::ToastManager()
{
}

ToastManager::~ToastManager() {
    for (QMap<int, Toaster*>::iterator it = _nfyStack.begin(); it != _nfyStack.end(); ++it)
        it.value()->deleteLater();
}

void ToastManager::add(Toaster *toast) {
    _nfyStackLock.lock();

    // Add a new notification window to the stack
    connect(toast, SIGNAL(notificationCanBeClosed(Toaster*)), this, SLOT(remove(Toaster*)));
    int toastNumber = _nfyStack.size();
    _nfyStack[toastNumber] = toast;
    moveToaster(toast, toastNumber);

    _nfyStackLock.unlock();

    // Activate the window
    toast->show();
}

void ToastManager::remove(Toaster *toast) {
    bool passedNfy = false;
    int lastKey = 0;

    // Reorder the notification stack accordingly
    _nfyStackLock.lock();
    QMap<int, Toaster*>::iterator it = _nfyStack.begin();

    // TODO: stacking produces unexpected results in
    // a test with 3 calendars generating notifications
    // simultaneously
    while (it != _nfyStack.end()) {
        Toaster* curToast = it.value();

        // Found notification window?
        if (curToast == toast) {
            passedNfy = true;
            toast->deleteLater();
            lastKey = it.key();
            it = _nfyStack.erase(it);
        }
        // All subsequent windows need to be moved to the front
        else if (passedNfy) {
            assert(!_nfyStack.contains(lastKey));
            _nfyStack[lastKey] = curToast;
            lastKey = it.key();
            it = _nfyStack.erase(it);
            moveToaster(curToast, lastKey);
        } else
            ++it;
    }

    _nfyStackLock.unlock();
}

void ToastManager::moveToaster(Toaster *toast, int position) {
    QDesktopWidget* qdw = QApplication::desktop();

    toast->setGeometry(qdw->availableGeometry().width() - Toaster::WIDTH,
                qdw->availableGeometry().height() - (Toaster::HEIGHT)*(position + 1),
                Toaster::WIDTH, Toaster::HEIGHT);
}
