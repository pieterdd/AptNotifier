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

This application and its source code is provided to you for free under a [Creative Commons BY-NC 2.0 license](http://creativecommons.org/licenses/by-nc/2.0/be/deed.en). Personal and commercial use of the application are allowed. You *may* use the source code (or parts thereof) in non-commercial works, provided that you release the source code of your application.

The work is provided to you as-is. Use this application and its source code at your own risk. I do not accept responsibility for any damages you or other parties may incur following your use of the work.

Acknowledgements
================

- [Gnome Project](http://www.iconfinder.com/icondetails/55237/48/48_appointment_gnome_soon_icon) for the system tray icon
- [Jack Cai](http://findicons.com/icon/177418/clock_red?id=333998#) for the calendar icon on the toaster messages