#include "stringres.h"

#ifndef DEBUG
const QString StringRes::_appName = "AptNotifier";
#else
const QString StringRes::_appName = "AptNotifier DEBUG";
#endif
const QString StringRes::_appVersion = "-- Test version";
