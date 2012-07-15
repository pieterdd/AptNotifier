#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QMutex>
#include <QString>
#include <QTextStream>

/**
  * Thread-safe class that logs various bits of information to a file.
  * @author Pieter De Decker
  */
class Logger
{
private:
    Logger();
public:
    static Logger* instance();

    /** [THREAD-SAFE] Writes a message to the log file. */
    void add(const QString& className, const QString &msg);
private:
    static Logger instancePtr;

    QFile _file;
    QMutex _fileLocker;
    QTextStream _fileStream;
};

#endif // LOGGER_H
