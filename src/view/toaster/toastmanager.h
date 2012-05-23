#ifndef TOASTMANAGER_H
#define TOASTMANAGER_H

#include <QMap>
#include <QMutex>
#include <QObject>
class Toaster;

/**
  * Manages all active toaster messages.
  * \author Pieter De Decker
  */
class ToastManager : public QObject
{
    Q_OBJECT
public:
    ToastManager();
    ~ToastManager();

    /** [THREAD-SAFE] Pop on a new toaster message. This class takes ownership of it. */
    void add(Toaster* toast);
public slots:
    /** [THREAD-SAFE] Closes and removes a certain toaster message. */
    void remove(Toaster* toast);
private:
    /** Updates the position of a certain toast message to reflect
      * its place in the stack. */
    void moveToaster(Toaster* toast, int position);

    QMap<int, Toaster*> _nfyStack;
    QMutex _nfyStackLock;
};

#endif // TOASTMANAGER_H
