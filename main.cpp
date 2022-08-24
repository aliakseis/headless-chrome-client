#include "mainwindow.h"
#include "echoclient.h"

#include <QApplication>

#include <QProcess>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>

#include <QSettings>

#include <iostream>

static QString getWebSocketDebuggerUrl()
{
    QNetworkAccessManager    networkAccessManager;
    QNetworkRequest request(QStringLiteral("http://localhost:9222/json/list"));

    QNetworkReply* reply = networkAccessManager.get(request);

    QEventLoop requrestWaitLoop;

    int redirectCount = 0;
    enum { REDIRECT_LIMIT = 10 };

    QByteArray syncResult;

    std::function<void()> finishedLam;
    finishedLam = [&reply, &redirectCount, &networkAccessManager, &finishedLam, &requrestWaitLoop, &syncResult] {
        QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if (!possibleRedirectUrl.isNull() && redirectCount++ < REDIRECT_LIMIT)
        {
            QUrl redirect = possibleRedirectUrl.toUrl();
            QUrl url = reply->url();
            reply->deleteLater();
            reply = networkAccessManager.get(QNetworkRequest(
                redirect.isRelative() ? url.scheme() + "://" + url.host() + redirect.toString() : redirect));

            QObject::connect(reply, &QNetworkReply::finished, finishedLam);
            //QObject::connect(reply, &QNetworkReply::NetworkError,
            //               reply, SLOT(onNetworkError(QNetworkReply::NetworkError))));
            return;
        }

        syncResult = reply->readAll();
        requrestWaitLoop.quit();
        reply->deleteLater();
        reply = nullptr;
    };

    QObject::connect(reply, &QNetworkReply::finished, finishedLam);

    requrestWaitLoop.exec();

    qDebug() << syncResult;

    auto doc = QJsonDocument::fromJson(syncResult);

    return doc[0]["webSocketDebuggerUrl"].toString();
}


int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: \"" << argv[0] << "\" url\n";
        return 1;
    }

    QApplication a(argc, argv);

    QProcess chromeProces;

    auto webSocketDebuggerUrl = getWebSocketDebuggerUrl();

    if (webSocketDebuggerUrl.isEmpty())
    {
        const auto split = QSettings(R"(HKEY_CLASSES_ROOT\ChromeHTML\shell\open\command)", QSettings::NativeFormat).value(".")
            .toString().split('"');

        if (split.length() < 2) {
            return 1;
        }

        const auto path = split[1];

        chromeProces.start(path, { "--remote-debugging-port=9222", "--headless" });
        if( !chromeProces.waitForStarted( 5000 ) )
        {
            // TODO Camera error message
            qDebug() << "Timed out starting a child process";
            return 1;
        }

        webSocketDebuggerUrl = getWebSocketDebuggerUrl();
    }

    EchoClient client(webSocketDebuggerUrl, argv[1], 1024, 600);
    QObject::connect(&client, &EchoClient::closed, &a, &QCoreApplication::quit);

    MainWindow w;

    QObject::connect(&client, &EchoClient::dataReceived, &w, &MainWindow::onDataReceived);

    w.show();
    return QApplication::exec();
}
