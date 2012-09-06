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

    /** Opens log file for writing and writes header. Must be called before writing
      * any log info! */
    void initialize();

    /** [THREAD-SAFE] Writes a message to the log file. */
    void add(const QString& className, const QString &msg);

    /** [THREAD-SAFE] Writes a message to the log file, appending
      * an object tag to keep track of the instance during the log. */
    void add(const QString& className, void* object, const QString& msg);

    /** Generates a 0xBAADF00D-style string for a given object. */
    static QString objectStr(void* object) {
        QString result;
        result.sprintf("%p", object);
        return result;
    }

    /** Generates full "(0xBAADF00D)" object tag. */
    static QString objectTag(void* object) {
        QString result;
        result.sprintf("(%p)", object);
        return result;
    }

private:
    static Logger instancePtr;

    QFile _file;
    QMutex _fileLocker;
    QTextStream _fileStream;
};

#endif // LOGGER_H
