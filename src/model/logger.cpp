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
    if (!_file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        std::cerr << "Couldn't open log file. Quitting.\n";
        exit(EXIT_FAILURE);
    }

    _fileStream << QString("\n== LOG DATE ") + QDateTime::currentDateTime().toString() + "\n";
    _fileStream << "This is " + QCoreApplication::applicationName() + " "
                   + QCoreApplication::applicationVersion() + " reporting for duty.\n";
}

void Logger::add(const QString& className, const QString& msg) {
    _fileLocker.lock();
    assert(_file.isOpen() && _file.isWritable());
    _fileStream << className + ": " + msg + "\n";
    _fileStream.flush();
    _fileLocker.unlock();
}
