#ifndef TOASTER_H
#define TOASTER_H

#include <QLabel>
#include <QString>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
class InfoWidget;

/**
  * General-purpose notification window.
  * \author Pieter De Decker
  */
class Toaster : public QDialog
{
    Q_OBJECT
public:
    /** \param infoWidget  Information provider. This dialog takes ownership. */
    Toaster(const QString& title, InfoWidget* infoWidget);
    ~Toaster();

    /** Override of the default show behavior to call InfoWidget's start()
      * function first. */
    void show();

    static const int WIDTH = 350;
    static const int HEIGHT = 150;
    static const int BORDERSPACING = 10;
private:
    /** Hooks up all widgets. */
    void setupGUI();

    QLabel _lblTitle;
    QVBoxLayout _vlMain;
    QHBoxLayout _hlBody;
    InfoWidget* _infoWidget;
protected:
    void paintEvent(QPaintEvent *);
private slots:
    /** Called when the InfoWidget is done presenting. */
    void receiveCloseMessage();
signals:
    /** Signals observers that this notification can be closed. */
    void notificationCanBeClosed(Toaster*);
};

#endif // TOASTER_H
