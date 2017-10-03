/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2016 Simon Stürz <simon.stuerz@guh.io>                   *
 *                                                                         *
 *  This file is part of guh.                                              *
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

#ifndef DEVICEPLUGINDOLLHOUSE_H
#define DEVICEPLUGINDOLLHOUSE_H

#include "plugin/deviceplugin.h"
#include "devicemanager.h"

#include "coap/coap.h"
#include "dollhouselight.h"

#include <QHash>

class DevicePluginDollHouse : public DevicePlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "guru.guh.DevicePlugin" FILE "deviceplugindollhouse.json")
    Q_INTERFACES(DevicePlugin)

public:
    DevicePluginDollHouse();

    DeviceManager::HardwareResources requiredHardware() const override;
    DeviceManager::DeviceSetupStatus setupDevice(Device *device) override;
    void deviceRemoved(Device *device) override;

    void guhTimer() override;
    DeviceManager::DeviceError executeAction(Device *device, const Action &action) override;

    void networkManagerReplyReady(QNetworkReply *reply) override;
private:
    Coap *m_coap;

    QHash<int, Device *> m_asyncSetup;
    QHash<QNetworkReply *, Device *> m_asyncNodeScan;

    QHash<CoapReply *, Action> m_asyncActions;
    QHash<ActionId, DollhouseLight *> m_asyncActionLights;

    QList<CoapReply *> m_asyncPings;

    QHostAddress m_houseAddress;
    bool m_houseReachable;

    QHash<Device *, DollhouseLight *> m_lights;

    void scanNodes(Device *device);
    void parseNode(Device *device, const QByteArray &data);

private slots:
    void hostLockupFinished(const QHostInfo &info);
    void coapReplyFinished(CoapReply *reply);

};

#endif // DEVICEPLUGINDOLLHOUSE_H