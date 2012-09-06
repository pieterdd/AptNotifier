#ifndef HTTPDOWNLOADER_H
#define HTTPDOWNLOADER_H

#include <QObject>
#include <QNetworkReply>
#include <QNetworkAccessManager>

/**
  * Assists in fetching HTTP resources. All functions in this class are thread-safe.
  *
  * @author Adapted from code published by FaddishWorm
  *   at http://stackoverflow.com/questions/12002947/possible-stack-corruption-during-use-of-qnetworkaccessmanager
  */
class HttpDownloader : public QObject
{
    Q_OBJECT
public:
    HttpDownloader(QObject *parent = 0);

    // Functions
    void doGet(const QString& url);
    void doGet(const QUrl& url);
    void doPost(const QString& url, QByteArray* message);
    void doPut(QString, QString);
    void doConnects(QNetworkReply* reply, QNetworkAccessManager* manager);
signals:
    void receivedData(bool success, QString* data);
private:
    static const char* CLASSNAME;
private slots:
    // Success slots
    void requestReturned(QNetworkReply* reply);

    // Failure slots
    void proxyAuthFail(const QNetworkProxy& proxy, QAuthenticator* authenticator);
    void sslErrorFail(QNetworkReply* reply, const QList<QSslError>& errors);
    void reqError(QNetworkReply::NetworkError code);
    void sslError(const QList<QSslError>& errors);
};

#endif // HTTPDOWNLOADER_H
