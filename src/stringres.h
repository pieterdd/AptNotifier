/**
  * StringRes
  * \author Pieter De Decker
  * \brief Globally used string resources.
  */
#ifndef STRINGRES_H
#define STRINGRES_H

#include <QString>

class StringRes {
public:
    static const QString& appName() { return _appName; }
    static const QString& appVersion() { return _appVersion; }
private:
    static const QString _appName;
    static const QString _appVersion;
};

#endif // STRINGRES_H
