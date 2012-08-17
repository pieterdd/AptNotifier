#include "logger.h"

#include <iostream>
#include <QDateTime>
#include <QCoreApplication>

Logger Logger::instancePtr;

Logger::Logger()
    : _file("log.txt"), _fileStream(&_file)
{
    if (!_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        std::cerr << "Couldn't open log file. Quitting.\n";
        exit(EXIT_FAILURE);
    }

    _fileStream << QString("\n== LOG DATE ") + QDateTime::currentDateTime().toString() + "\n";
    _fileStream << "This is " + QCoreApplication::applicationName() + " "
                   + QCoreApplication::applicationVersion() + " reporting for duty.\n";
}

Logger* Logger::instance() {
    return &instancePtr;
}

void Logger::add(const QString& className, const QString& msg) {
    _fileLocker.lock();
    _fileStream << className + ": " + msg + "\n";
    _fileStream.flush();
    _fileLocker.unlock();
}
