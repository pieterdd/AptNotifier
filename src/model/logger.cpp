#include "logger.h"

#include <cassert>
#include <iostream>
#include <QDateTime>
#include <QCoreApplication>

Logger Logger::instancePtr;

Logger::Logger()
    : _file("log.txt"), _fileStream(&_file)
{}

Logger* Logger::instance() {
    return &instancePtr;
}

void Logger::initialize() {
    bool fileExists = _file.exists() && (_file.size() > 0);
    if (!_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        std::cerr << "Couldn't open log file. Quitting.\n";
        exit(EXIT_FAILURE);
    }

    if (fileExists)
        _fileStream << "\n";
    _fileStream << QString("== LOG DATE ") + QDateTime::currentDateTime().toString() + "\n";
    _fileStream << "This is " + QCoreApplication::applicationName() + " "
                   + QCoreApplication::applicationVersion() + " reporting for duty.\n";
}

void Logger::add(const QString& className, const QString& msg) {
    Q_UNUSED(className)
    Q_UNUSED(msg)
#ifdef DEBUG
    _fileLocker.lock();
    assert(_file.isOpen() && _file.isWritable());
    _fileStream << "[" + QDateTime::currentDateTime().toString("hh:mm:ss") + "] " + className + ": " + msg + "\n";
    _fileStream.flush();
    _fileLocker.unlock();
#endif
}

void Logger::add(const QString& className, void* object, const QString& msg) {
    add(className, msg + " " + objectTag(object));
}
