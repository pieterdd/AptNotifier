#ifndef STRINGRES_H
#define STRINGRES_H

#include <QString>

/**
  * Globally used string resources.
  * \author Pieter De Decker
  */
class StringRes {
public:
    static const QString& appName() { return _appName; }
    static const QString& appVersion() { return _appVersion; }
private:
    static const QString _appName;
    static const QString _appVersion;
};

#endif // STRINGRES_H
