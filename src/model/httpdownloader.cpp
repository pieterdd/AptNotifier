#include "httpdownloader.h"

#include "logger.h"
#include <QUrl>
#include <cassert>
#include <QNetworkRequest>

const char* HttpDownloader::CLASSNAME = "HttpDownloader";

HttpDownloader::HttpDownloader(QObject *parent) :
    QObject(parent)
{
}

void HttpDownloader::doGet(const QString& url) {
    QByteArray urlArray;
    QUrl urlObj = QUrl::fromEncoded(urlArray.append(url));
    doGet(urlObj);
}

void HttpDownloader::doGet(const QUrl &url) {
    Logger::instance()->add(CLASSNAME, this, "Filed GET request for " + url.toString());
    QNetworkRequest request(url);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->get(request);
    doConnects(reply, manager);
}

void HttpDownloader::doPost(const QString& url, QByteArray *message) {
    assert(message);
    Logger::instance()->add(CLASSNAME, this, "Filed POST request for " + url);
    QNetworkRequest request(url);
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, *message);
    doConnects(reply, manager);
}

void HttpDownloader::doConnects(QNetworkReply *reply, QNetworkAccessManager* manager){
    assert(reply);
    assert(manager);

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


void HttpDownloader::requestReturned(QNetworkReply* rep) {
    assert(rep);
    QVariant status = rep->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (status != 200 || status == NULL) {
        Logger::instance()->add(CLASSNAME, this, "Received non-200 response");
        QString *lastError = new QString("HTTP ERROR " + rep->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString()
                                         + " (" + rep->errorString() + ")\r\n---\r\n\r\n " + rep->readAll());

#ifdef DEBUG
        // TODO DEBUG: dump error page
        QFile file("test");
        if (!file.open(QIODevice::WriteOnly))
            return;
        QTextStream out(&file);
        out << *lastError;
        file.close();
#endif

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
    Logger::instance()->add(CLASSNAME, this, "Request QNetworkReply::NetworkError code " + QString::number(code));
}

void HttpDownloader::sslError(const QList<QSslError>&) {
    Logger::instance()->add(CLASSNAME, this, "SSL error in QNetworkReply");
}
