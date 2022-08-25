/****************************************************************************
**
** Copyright (C) 2016 Kurt Pattyn <pattyn.kurt@gmail.com>.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtWebSockets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/
#include "echoclient.h"
#include <QtCore/QDebug>

#include <QJsonDocument>
#include <QByteArray>

#include <QTimer>

template<typename T>
auto num(T v)
{
    return QString::number(v);
}

#define S(...) QStringLiteral(#__VA_ARGS__)

//! [constructor]
EchoClient::EchoClient(const QUrl& webSocketUrl, const QString &url,
                       int width, int height, bool debug, QObject *parent) :
    QObject(parent),
    m_url(url),
    m_debug(debug),
    m_width(width),
    m_height(height)
{
    if (m_debug) {
        qDebug() << "WebSocket server:" << url;
    }
    connect(&m_webSocket, &QWebSocket::connected, this, &EchoClient::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &EchoClient::closed);
    m_webSocket.open(webSocketUrl);
}
//! [constructor]

//! [onConnected]
void EchoClient::onConnected()
{
    if (m_debug) {
        qDebug() << "WebSocket connected";
    }
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &EchoClient::onTextMessageReceived);
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this, &EchoClient::onBinaryMessageReceived);

    m_webSocket.sendTextMessage(S({ "id": 1, "method": "Page.enable", "params": {} }));
    m_webSocket.sendTextMessage(
        S({ "id": 2, "method": "Page.setDeviceMetricsOverride", "params": { "width" : ) + num(m_width)
        + S(, "height" : ) + num(m_height) + S(, "deviceScaleFactor" : 1, "mobile" : false } }));
    m_webSocket.sendTextMessage(S({ "id": 3, "method": "Page.startScreencast", "params": {} }));
    m_webSocket.sendTextMessage(
        S({ "id": 4, "method": "Page.navigate", "params": { "url": ) + '"' + m_url + '"' + S( } }));
    m_id = 5;
}
//! [onConnected]


void EchoClient::onClosed()
{
    qDebug() << "closed";
}


//! [onTextMessageReceived]
void EchoClient::onTextMessageReceived(const QString& message)
{
    QByteArray ba = message.toUtf8();

    auto doc = QJsonDocument::fromJson(ba);

    /*
    auto ret_id = doc["id"];
    if (!ret_id.isNull())
    {
        auto v = ret_id.toInt();
        if (v != 0 && v == m_pressedId)
        {
            const auto x = QStringLiteral("75");//num(deviceWidth / 2);
            const auto y = QStringLiteral("460");//num(deviceHeight / 2 - 30);

            //m_webSocket.sendTextMessage(
            //    S({ "id": ) + num(m_id++) + S(, "method": "Input.dispatchMouseEvent",
            //        "params": { "type" : "mousePressed", "x" : ) + x + S(, "y" : ) + y + S(, "button" : "left" } }));
            m_webSocket.sendTextMessage(
                S({ "id":) + num(m_id++) + S(, "method": "Input.dispatchMouseEvent",
                    "params" : { "type" : "mouseReleased", "x" : ) + x + S(, "y" :) + y + S(, "button" : "left" } }));

            m_pressedId = 0;

            return;
        }
    }
    */

    QString docMethod = doc["method"].toString();
    if (docMethod == "Page.screencastFrame")
    {        
        if (m_debug) {
            qDebug() << "Message received start: " << message.left(1000);
            qDebug() << "Message received end: " << message.right(1000);
        }

        const auto params = doc["params"];
        const auto data = params["data"].toString();
        const auto bin = QByteArray::fromBase64(data.toLatin1());
        emit dataReceived(bin);

        const auto sessionId = params["sessionId"].toInt();
        m_webSocket.sendTextMessage(
            S({ "id":) + num(m_id++) + S(, "method": "Page.screencastFrameAck", "params" : { "sessionId":) 
            + num(sessionId) + S(} }));

        //if (m_pressed) // looks like a double click
        {
            // \"metadata\":{\"offsetTop\":0,\"pageScaleFactor\":1,\"deviceWidth\":800,\"deviceHeight\":600,\"scrollOffsetX\":0,\"scrollOffsetY\":0,\"timestamp\":1661238724.105544}
            /*
            const auto metadata = params["metadata"];
            const auto deviceWidth = metadata["deviceWidth"].toInt();
            const auto deviceHeight = metadata["deviceHeight"].toInt();
            */

            /*
            const auto x = QStringLiteral("75");//num(deviceWidth / 2);
            const auto y = QStringLiteral("460");//num(deviceHeight / 2 - 30);

            //m_webSocket.sendTextMessage(
            //    S({ "id": ) + num(m_id++) + S(, "method": "Input.dispatchMouseEvent",
            //        "params": { "type" : "mousePressed", "x" : ) + x + S(, "y" : ) + y + S(, "button" : "left" } }));
            m_webSocket.sendTextMessage(
                S({ "id": ) + num(m_id++) + S(, "method": "Input.dispatchMouseEvent",
                    "params": { "type" : "mouseReleased", "x" : ) + x + S(, "y" : ) + y + S(, "button" : "left" } }));
            
            m_pressed = false;
            */
        }
    }
    else if (docMethod == "Page.frameStoppedLoading")
    {
        if (m_debug) {
            qDebug() << "Message received:" << message;
        }

        const auto x = QStringLiteral("75");//num(deviceWidth / 2);
        const auto y = QStringLiteral("460");//num(deviceHeight / 2 - 30);

        //m_pressedId = m_id;

        //*

        m_webSocket.sendTextMessage(
            S({ "id":) + num(m_id++) + S(, "method": "Input.dispatchMouseEvent",
                "params" : { "type" : "mousePressed", "x" : ) + x + S(, "y" :) + y + S(, "button" : "left", "clickCount" : 1 } }));
        m_webSocket.sendTextMessage(
            S({ "id":) + num(m_id++) + S(, "method": "Input.dispatchMouseEvent",
                "params" : { "type" : "mouseReleased", "x" : ) + x + S(, "y" :) + y + S(, "button" : "left", "clickCount" : 1 } }));

        /*
        QMetaObject::invokeMethod(this, [this]
            {
                const auto x = QStringLiteral("75");//num(deviceWidth / 2);
                const auto y = QStringLiteral("460");//num(deviceHeight / 2 - 30);

                m_webSocket.sendTextMessage(
                    S({ "id":) + num(m_id++) + S(, "method": "Input.dispatchMouseEvent",
                        "params" : { "type" : "mouseReleased", "x" : ) + x + S(, "y" :) + y + S(, "button" : "left" } }));
            }, Qt::QueuedConnection);


        */

        /*

        //QMetaObject::invokeMethod(this, [this]
        if (!m_clicked)
        {
            m_clicked = true;

            QTimer::singleShot(5000, [this]
                {
                    qDebug() << "*** singleShot fired! ***\n";

                    const auto x = QStringLiteral("75");//num(deviceWidth / 2);
                    const auto y = QStringLiteral("460");//num(deviceHeight / 2 - 30);

                    m_webSocket.sendTextMessage(
                        S({ "id":) + num(m_id++) + S(, "method": "Input.dispatchMouseEvent",
                            "params" : { "type" : "mousePressed", "x" : ) + x + S(, "y" :) + y + S(, "button" : "left", "clickCount" : 1 } }));


                    //QMetaObject::invokeMethod(this, [this]
                    QTimer::singleShot(500, [this]
                        {
                            const auto x = QStringLiteral("75");//num(deviceWidth / 2);
                            const auto y = QStringLiteral("460");//num(deviceHeight / 2 - 30);

                            m_webSocket.sendTextMessage(
                                S({ "id":) + num(m_id++) + S(, "method": "Input.dispatchMouseEvent",
                                    "params" : { "type" : "mouseReleased", "x" : ) + x + S(, "y" :) + y + S(, "button" : "left", "clickCount" : 1 } }));




                        });// , Qt::QueuedConnection);


                });// , Qt::QueuedConnection);
        }

        */

    }
    else
    {
        if (m_debug) {
            qDebug() << "Message received:" << message;
        }
    }
    //m_webSocket.close();
}
//! [onTextMessageReceived]

void EchoClient::onBinaryMessageReceived(const QByteArray& message)
{
    if (m_debug) {
        qDebug() << "Message received:" << message;
    }
    //m_webSocket.close();
}
