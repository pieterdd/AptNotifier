#-------------------------------------------------
#
# Project created by QtCreator 2012-02-04T14:21:56
#
#-------------------------------------------------

QT       += core gui network

TARGET = AptNotifier
TEMPLATE = app


SOURCES += main.cpp\
    model/appointment.cpp \
    model/calendar.cpp \
    model/calendardb.cpp \
    view/calendardbview.cpp \
    view/aptnotification.cpp \
    view/inputbox.cpp \
    stringres.cpp \
    view/toaster/toaster.cpp \
    view/toaster/appointmentlist.cpp \
    view/toaster/infowidget.cpp \
    view/toaster/toastmanager.cpp

HEADERS  += \
    model/appointment.h \
    model/calendar.h \
    model/calendardb.h \
    view/calendardbview.h \
    view/aptnotification.h \
    view/inputbox.h \
    stringres.h \
    view/toaster/toaster.h \
    view/toaster/appointmentlist.h \
    view/toaster/infowidget.h \
    view/toaster/toastmanager.h

RESOURCES += \
    resources.qrc

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}
