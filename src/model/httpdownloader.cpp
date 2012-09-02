#include "httpdownloader.h"

#include "logger.h"
#include <QUrl>
#include <QNetworkRequest>

const char* HttpDownloader::CLASSNAME = "HttpDownloader";

HttpDownloader::HttpDownloader(QObject *parent) :
    QObject(parent)
{
}

void HttpDownloader::doGet(const QString& url) {
    Logger::instance()->add(CLASSNAME, this, "Filed GET request for " + url);
    QUrl requestedUrl(url);
    QNetworkRequest request(requestedUrl);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);
    doConnects(reply, manager);
}

void HttpDownloader::doPost(const QString& url, QByteArray *message) {
    Logger::instance()->add(CLASSNAME, this, "Filed POST request for " + url);
    QNetworkRequest request(url);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, *message);
    doConnects(reply, manager);
}

void HttpDownloader::doConnects(QNetworkReply *reply, QNetworkAccessManager* manager){
    // Reply connects
    QObject::connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
                     this, SLOT(reqError(QNetworkReply::NetworkError)));
    QObject::connect(reply, SIGNAL(sslErrors(QList<QSslError>)),
                     this, SLOT(sslError(QList<QSslError>)));

    // Manager connects
    QObject::connect(manager, SIGNAL(finished(QNetworkReply*)),
               this, SLOT(requestReturned(QNetworkReply*)));
    QObject::connect(manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),
            this, SLOT(sslErrorFail(QNetworkReply*,QList<QSslError>)));
    QObject::connect(manager, SIGNAL(proxyAuthenticationRequired(QNetworkProxy,QAuthenticator*)),
               this, SLOT(proxyAuthFail(QNetworkProxy,QAuthenticator*)));
}


void HttpDownloader::requestReturned(QNetworkReply* rep){
    QVariant status = rep->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if(status != 200 || status == NULL) {
        Logger::instance()->add(CLASSNAME, this, "Received non-200 response");
        QString *lastError = new QString("ERROR: " + status.toString()
                                         + " " + rep->readAll());
        emit receivedData(false, lastError);
        delete lastError;
    } else {
        Logger::instance()->add(CLASSNAME, this, "Received response");
        QString *retString = new QString(rep->readAll());
        *retString = retString->trimmed();
        emit receivedData(true, retString);
        delete retString;
    }

    rep->manager()->deleteResource(rep->request());
    rep->manager()->deleteLater();
    rep->deleteLater();
    sender()->deleteLater();
}

void HttpDownloader::proxyAuthFail(const QNetworkProxy&, QAuthenticator*) {
    Logger::instance()->add(CLASSNAME, this, "Proxy authentication failed");
}

void HttpDownloader::sslErrorFail(QNetworkReply*, const QList<QSslError>&) {
    Logger::instance()->add(CLASSNAME, this, "SSL error in QNetworkManager");
}

void HttpDownloader::reqError(QNetworkReply::NetworkError code) {
    Logger::instance()->add(CLASSNAME, this, "Request error code 0x" + QString::number(16, code));
}

void HttpDownloader::sslError(const QList<QSslError>&) {
    Logger::instance()->add(CLASSNAME, this, "SSL error in QNetworkReply");
}
