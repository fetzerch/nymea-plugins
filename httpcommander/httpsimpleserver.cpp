/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2015 Simon Stürz <simon.stuerz@guh.io>                   *
 *  Copyright (C) 2014 Michael Zanetti <michael_zanetti@gmx.net>           *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  This library is free software; you can redistribute it and/or          *
 *  modify it under the terms of the GNU Lesser General Public             *
 *  License as published by the Free Software Foundation; either           *
 *  version 2.1 of the License, or (at your option) any later version.     *
 *                                                                         *
 *  This library is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *  Lesser General Public License for more details.                        *
 *                                                                         *
 *  You should have received a copy of the GNU Lesser General Public       *
 *  License along with this library; If not, see                           *
 *  <http://www.gnu.org/licenses/>.                                        *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "httpsimpleserver.h"

#include "types/statetype.h"
#include "extern-plugininfo.h"

#include <QTcpSocket>
#include <QDebug>
#include <QDateTime>
#include <QUrlQuery>
#include <QRegExp>
#include <QStringList>

HttpSimpleServer::HttpSimpleServer(quint16 port, QObject *parent):
    QTcpServer(parent)
{
    listen(QHostAddress::Any, port);
}

HttpSimpleServer::~HttpSimpleServer()
{
    close();
}

void HttpSimpleServer::incomingConnection(qintptr socket)
{
    // When a new client connects, the server constructs a QTcpSocket and all
    // communication with the client is done over this QTcpSocket. QTcpSocket
    // works asynchronously, this means that all the communication is done
    // in the two slots readClient() and discardClient().
    QTcpSocket* tcpSocket = new QTcpSocket(this);
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(discardClient()));
    tcpSocket->setSocketDescriptor(socket);

}

void HttpSimpleServer::readClient()
{
    // This slot is called when the client sent data to the server. The
    // server looks if it was a get request and sends a very simple HTML
    // document back.
    QTcpSocket* tcpSocket = static_cast<QTcpSocket*>(sender());
    if (tcpSocket->canReadLine()) {

        QByteArray data = tcpSocket->readAll();
        QStringList tokens = QString(data).split(QRegExp("[ \r\n][ \r\n]*"));
        qCDebug(dcHttpCommander()) << "Http Request, type" << tokens[0] << "path" << tokens[1] << "body" << tokens.last();

        if ((tokens[0] == "GET")      ||
                (tokens[0] == "PUT")  ||
                (tokens[0] == "POST") ||
                (tokens[0] == "DELETE")) {

            QTextStream os(tcpSocket);
            os.setAutoDetectUnicode(true);
            os << generateHeader();
            tcpSocket->close();

            if (tcpSocket->state() == QTcpSocket::UnconnectedState)
                delete tcpSocket;

            emit requestReceived(tokens[0], tokens[1], tokens.last());
        }
    }
}

void HttpSimpleServer::discardClient()
{
    QTcpSocket* socket = static_cast<QTcpSocket*>(sender());
    socket->deleteLater();
}

QString HttpSimpleServer::generateHeader()
{
    QString contentHeader(
                "HTTP/1.0 200 Ok\r\n"
                "Content-Type: text/html; charset=\"utf-8\"\r\n"
                "\r\n"
                );
    return contentHeader;
}