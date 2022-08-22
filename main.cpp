#include "mainwindow.h"
#include "echoclient.h"

#include <QApplication>

#include <QProcess>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#include <QJsonDocument>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QProcess mGstProcess;

    mGstProcess.start( "C:/Progra~2/Google/Chrome/Application/chrome.exe --remote-debugging-port=9222 --headless" );
    if( !mGstProcess.waitForStarted( 5000 ) )
    {
        // TODO Camera error message
        qDebug() << "Timed out starting a child process";
        return 1;
    }


    QNetworkAccessManager    networkAccessManager;
    QNetworkRequest request(QStringLiteral("http://localhost:9222/json/list"));

    QNetworkReply* reply = networkAccessManager.get(request);

    QEventLoop m_requrestWaitLoop;

    int m_redirectCount = 0;
    enum { REDIRECT_LIMIT = 10 };

    QByteArray syncResult;

    std::function<void()> finishedLam;
    finishedLam = [&reply, &m_redirectCount, &networkAccessManager, &finishedLam, &m_requrestWaitLoop, &syncResult] {
        QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);

        if (!possibleRedirectUrl.isNull() && m_redirectCount++ < REDIRECT_LIMIT)
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
        m_requrestWaitLoop.quit();
        reply->deleteLater();
        reply = nullptr;
    };

    QObject::connect(reply, &QNetworkReply::finished, finishedLam);

    m_requrestWaitLoop.exec();

    qDebug() << syncResult;

    auto doc = QJsonDocument::fromJson(syncResult);

    auto webSocketDebuggerUrl = doc[0]["webSocketDebuggerUrl"].toString();

    bool debug = true;
    EchoClient client(QUrl(webSocketDebuggerUrl), debug);
    QObject::connect(&client, &EchoClient::closed, &a, &QCoreApplication::quit);

    MainWindow w;

    QObject::connect(&client, &EchoClient::dataReceived, &w, &MainWindow::onDataReceived);

    w.show();
    return a.exec();
}
