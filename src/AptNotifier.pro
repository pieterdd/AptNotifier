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
    view/inputbox.cpp \
    view/toaster/toaster.cpp \
    view/toaster/toastmanager.cpp \
    model/logger.cpp \
    model/aptcache.cpp \
    model/httpdownloader.cpp \
    model/icsparser.cpp \
    view/toaster/aptbundle.cpp \
    view/toaster/aptdisplaywidget.cpp

HEADERS  += \
    model/appointment.h \
    model/calendar.h \
    model/calendardb.h \
    view/calendardbview.h \
    view/inputbox.h \
    view/toaster/toaster.h \
    view/toaster/toastmanager.h \
    model/logger.h \
    model/aptcache.h \
    model/httpdownloader.h \
    model/icsparser.h \
    view/toaster/aptbundle.h \
    view/toaster/aptdisplaywidget.h

RESOURCES += \
    resources.qrc

CONFIG(debug, debug|release) {
    DEFINES += DEBUG
}

win32:RC_FILE = AptNotifier.rc
