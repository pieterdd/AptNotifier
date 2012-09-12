#ifndef TOASTER_H
#define TOASTER_H

#include "aptbundle.h"
#include "aptdisplaywidget.h"
#include <QLabel>
#include <QTimer>
#include <QString>
#include <QDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLinkedList>

// TODO: fix skip to 2nd appointment,
// deprecate old classes.

/**
  * General purpose notification window. This class is NOT thread-safe.
  * \author Pieter De Decker
  */
class Toaster : public QDialog
{
    Q_OBJECT
public:
    Toaster();
    ~Toaster();

    /** Adds new appointments to this toaster. */
    void appendBundle(const AptBundle& bundle);

    /** Override of the default show behavior to start the slide timer. */
    void show();

    static const int WIDTH = 350;
    static const int HEIGHT = 150;
    static const int BORDERSPACING = 10;
private:
    static const char* CLASSNAME;

    /** Hooks up all widgets. */
    void setupGUI();

    /** Calculates the total number of appointments in the queue. */
    int totalApts() const;

    /** Loads the appointment from (bundle ID, appointment ID) into the toaster.
      * The accompanying title and calendar indicator are also updated. */
    void loadAppointment();

    /** Returns true if a bundle with ID 'bundleID' exists *and* this bundle
      * holds an appointment with ID 'aptID'. */
    bool exists(int bundleID, int aptID);

    // Widgets and layouts
    QLabel _lblTitle;
    AptDisplayWidget _adg;
    QLabel _lblCounter;
    QPushButton _btnPrev;
    QPushButton _btnNext;
    QVBoxLayout _vlMain;
    QHBoxLayout _hlBody;
    QVBoxLayout _vlInfo;
    QHBoxLayout _hlBottom;

    QTimer _slideTimer;
    QList<AptBundle> _bundles;
    int _curBundle;
    int _curApt;
    int _curAptID;
private slots:
    /** Advances to the previous slide. If unavailable, does nothing. */
    void prevSlide();

    /** Advances to the next slide. If unavailable, sends a close signal.
      * This function is also triggered whenever the slide timer times out.
      * Because of the way connect()'s AutoConnection works, thread sync
      * is not needed. */
    void nextSlide();
protected:
    void paintEvent(QPaintEvent*);
signals:
    /** Signals observers that this notification can be closed. */
    void closeRequested(Toaster*);
};

#endif // TOASTER_H
