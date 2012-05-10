#include "toaster.h"

#include "view/toaster/infowidget.h"
#include <QFont>
#include <cassert>
#include <QPainter>
#include <QSpacerItem>

Toaster::Toaster(const QString &title, InfoWidget *infoWidget)
    : QDialog(NULL)
{
    assert(infoWidget);
    _infoWidget = infoWidget;
    _lblTitle.setText(title);
    setupGUI();
}

Toaster::~Toaster() {
    delete _infoWidget;
}

void Toaster::show() {
    assert(_infoWidget);
    _infoWidget->start();
    QDialog::show();
}

void Toaster::setupGUI() {
    setContentsMargins(0, 0, 0, 0);
    setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(WIDTH, HEIGHT);

    _vlMain.setContentsMargins(0, 0, 0, 0);
    _lblTitle.setContentsMargins(15, 0, 0, 0);
    _lblTitle.setStyleSheet("color: #fff; font-family: Arial; font-size: 16px; font-weight: bold");
    _lblTitle.setFixedHeight(35);
    _vlMain.addWidget(&_lblTitle);
    _vlMain.addLayout(&_hlBody);
    _vlMain.addSpacerItem(new QSpacerItem(0, 5));
    _hlBody.addSpacerItem(new QSpacerItem(100, 0));
    _hlBody.addWidget(_infoWidget);
    _hlBody.addSpacerItem(new QSpacerItem(5, 0));
    setLayout(&_vlMain);
    connect(_infoWidget, SIGNAL(doneShowingInfo()), this, SLOT(receiveCloseMessage()));
}

void Toaster::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.drawImage(0, 0, QImage(":/bg/toaster.png"));
}

void Toaster::receiveCloseMessage()
{
    emit notificationCanBeClosed(this);
}
