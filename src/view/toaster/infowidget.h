#ifndef INFOCOMPONENT_H
#define INFOCOMPONENT_H

#include <QWidget>

/**
  * Abstract base class for all information widgets that are
  * are to be used in conjunction with a toaster window.
  * \author Pieter De Decker
  */
class InfoWidget : public QWidget
{
    Q_OBJECT
public:
    InfoWidget();

    /** Multi-slide info widgets can use this as a starting point
      * for their cycle timers. */
    virtual void start() = 0;
signals:
    /** Should be emitted when the info widget is done presenting
      * data to gently request the toaster message to close. */
    void doneShowingInfo();
};

#endif // INFOCOMPONENT_H
