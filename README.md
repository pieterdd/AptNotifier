Description
===========

AptNotifier (pronounced *Appointment Notifier*) is an application that helps you keep track of your calendar by notifying you about upcoming and ongoing events. It is written in C++ and depends on the Qt library. Since Qt is available for all major platforms, AptNotifier works on Windows, Linux and Mac.

For an up-to-date list of the current limitations of AptNotifier, check our [issue tracker](https://github.com/pieterdd/AptNotifier/issues).


Compiling with MinGW (Windows)
==============================

I've provided a PRO file for AptNotifier that can be opened with Qt Creator. If you're compiling Qt Creator with MinGW GCC on Windows, you'll need the following DLLs:

- **Qt DLLs**: QtCore4.dll, QtGui4.dll, QtNetwork4.dll
- **MinGW**: libgcc_s_dw2-1.dll, mingwm10.dll
- **For SSL support**: libeay32.dll, ssleay32.dll

If the executable can't find the DLLs necessary for SSL support, calendar URLs that use HTTPS may fail to load silently.


Compiling on Linux and Mac
==========================

Although AptNotifier should be able to run on any platform with a Qt desktop implementation, I haven't tested it on Linux and Mac. I would suggest tinkering with *ldd* to find out which shared objects are essential to the execution of the application.


License
=======

See the LICENSE file for more information about the terms under which AptNotifier is licensed.

Acknowledgements
================

- [Gnome Project](http://www.iconfinder.com/icondetails/55237/48/48_appointment_gnome_soon_icon) for the system tray icon
- [Jack Cai](http://findicons.com/icon/177418/clock_red?id=333998#) for the calendar icon on the toaster messages