/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by
* copyright law, and remains the property of nymea GmbH. All rights, including
* reproduction, publication, editing and translation, are reserved. The use of
* this project is subject to the terms of a license agreement to be concluded
* with nymea GmbH in accordance with the terms of use of nymea GmbH, available
* under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the
* terms of the GNU Lesser General Public License as published by the Free
* Software Foundation; version 3. This project is distributed in the hope that
* it will be useful, but WITHOUT ANY WARRANTY; without even the implied
* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this project. If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under
* contact@nymea.io or see our FAQ/Licensing Information on
* https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef HOMECONNECT_H
#define HOMECONNECT_H

#include <QObject>
#include <QTimer>

#include "network/networkaccessmanager.h"

class HomeConnect : public QObject
{
    Q_OBJECT
public:
    HomeConnect(NetworkAccessManager *networkmanager,  const QByteArray &clientKey,  const QByteArray &clientSecret, QObject *parent = nullptr);

    QUrl getLoginUrl(const QUrl &redirectUrl, const QString &scope);
    void checkStatusCode(int status, const QByteArray &payload);
    void getAccessTokenFromRefreshToken(const QByteArray &refreshToken);
    void getAccessTokenFromAuthorizationCode(const QByteArray &authorizationCode);

private:
    QByteArray m_baseAuthorizationUrl = "https://api.home-connect.com/security/oauth/authorize";
    QByteArray m_baseTokenUrl = "https://api.home-connect.com/security/oauth/token";
    QByteArray m_baseControlUrl = "https://api.home-connect.com";
    QByteArray m_clientKey;
    QByteArray m_clientSecret;

    QByteArray m_accessToken;
    QByteArray m_refreshToken;
    QByteArray m_redirectUri;

    NetworkAccessManager *m_networkManager = nullptr;
    QTimer *m_tokenRefreshTimer = nullptr;

private slots:
    void onRefreshTimeout();

signals:
    void connectionChanged(bool connected);
    void authenticationStatusChanged(bool authenticated);
    void actionExecuted(QUuid actionId,bool success);
};
#endif // HOMECONNECT_H
