/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2017 Bernhard Trinnes <bernhard.trinnes@guh.io>          *
 *  Copyright (C) 2018 Simon Stürz <simon.stuerz@guh.io>                   *
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

#include "devicepluginunipi.h"
#include "plugininfo.h"
#include <QJsonDocument>

DevicePluginUniPi::DevicePluginUniPi()
{

}

DevicePluginUniPi::~DevicePluginUniPi()
{

}

void DevicePluginUniPi::init()
{
    if (m_webSocket == NULL) {

        int port = 8080; //configValue(uniPiPortParamTypeId).toInt();

        m_webSocket = new QWebSocket();
        connect(m_webSocket, &QWebSocket::connected, this, &DevicePluginUniPi::onWebSocketConnected);
        connect(m_webSocket, &QWebSocket::disconnected, this, &DevicePluginUniPi::onWebSocketDisconnected);

        QUrl url = QUrl("ws://localhost/ws");
        url.setPort(port);
        qCDebug(dcUniPi()) << "Conneting to:" << url.toString();
        m_webSocket->open(url);
    }
}

DeviceManager::DeviceSetupStatus DevicePluginUniPi::setupDevice(Device *device)
{
    if (device->deviceClassId() == relayOutputDeviceClassId) {

        m_usedRelais.insert(device->paramValue(relayOutputRelayNumberParamTypeId).toString(), device);
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    if (device->deviceClassId() == digitalOutputDeviceClassId) {

        m_usedDigitalOutputs.insert(device->paramValue(digitalOutputDigitalOutputNumberParamTypeId).toString(), device);
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    if (device->deviceClassId() == digitalInputDeviceClassId) {

        m_usedDigitalInputs.insert(device->paramValue(digitalInputDigitalInputNumberParamTypeId).toString(), device);
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    if (device->deviceClassId() == analogInputDeviceClassId) {

        m_usedAnalogInputs.insert(device->paramValue(analogInputAnalogInputNumberParamTypeId).toString(), device);
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    if (device->deviceClassId() == analogOutputDeviceClassId) {

        m_usedAnalogOutputs.insert(device->paramValue(analogOutputAnalogOutputNumberParamTypeId).toString(), device);
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    if (device->deviceClassId() == shutterDeviceClassId) {

        if (device->paramValue(shutterOutputTypeOpenParamTypeId) == GPIOType::relay) {
            m_usedRelais.insert(device->paramValue(shutterOutputOpenParamTypeId).toString(), device);
        } else if (device->paramValue(shutterOutputOpenParamTypeId) == GPIOType::digitalOutput) {
            m_usedDigitalOutputs.insert(device->paramValue(shutterOutputOpenParamTypeId).toString(), device);
        }

        if (device->paramValue(shutterOutputTypeCloseParamTypeId) == GPIOType::relay) {
            m_usedRelais.insert(device->paramValue(shutterOutputCloseParamTypeId).toString(), device);
        } else if (device->paramValue(shutterOutputOpenParamTypeId) == GPIOType::digitalOutput) {
            m_usedDigitalOutputs.insert(device->paramValue(shutterOutputCloseParamTypeId).toString(), device);
        }

        return DeviceManager::DeviceSetupStatusSuccess;
    }

    if (device->deviceClassId() == lightDeviceClassId) {

        if (device->paramValue(lightOutputTypeParamTypeId) == GPIOType::relay) {
            m_usedRelais.insert(device->paramValue(lightOutputParamTypeId).toString(), device);
        } else if (device->paramValue(lightOutputParamTypeId) == GPIOType::digitalOutput) {
            m_usedDigitalOutputs.insert(device->paramValue(lightOutputParamTypeId).toString(), device);
        }
        return DeviceManager::DeviceSetupStatusSuccess;
    }

    return DeviceManager::DeviceSetupStatusFailure;
}

DeviceManager::DeviceError DevicePluginUniPi::discoverDevices(const DeviceClassId &deviceClassId, const ParamList &params)
{
    Q_UNUSED(params);

    const DeviceClass deviceClass = deviceManager()->findDeviceClass(deviceClassId);
    if (deviceClass.vendorId() == unipiVendorId) {

        if (deviceClassId == relayOutputDeviceClassId) {
            // Create the list of available relais
            QList<DeviceDescriptor> deviceDescriptors;
            for (int i = 0; i < m_relais.count(); i++) {
                const QString circuit = m_relais.at(i);

                // Offer only gpios which arn't in use already
                if (m_usedRelais.contains(circuit)){
                    continue;
                }
                DeviceDescriptor descriptor(deviceClassId, QString("Relay %1").arg(circuit), circuit);
                ParamList parameters;
                parameters.append(Param(relayOutputRelayNumberParamTypeId, circuit));

                descriptor.setParams(parameters);
                deviceDescriptors.append(descriptor);
            }
            emit devicesDiscovered(deviceClassId, deviceDescriptors);
            return DeviceManager::DeviceErrorAsync;
        }

        if (deviceClassId == digitalOutputDeviceClassId) {
            // Create the list of available digital outputs
            QList<DeviceDescriptor> deviceDescriptors;
            for (int i = 0; i < m_digitalOutputs.count(); i++) {
                const QString circuit = m_digitalOutputs.at(i);

                // Offer only gpios which arn't in use already
                if (m_usedDigitalOutputs.contains(circuit)){
                    continue;
                }
                DeviceDescriptor descriptor(deviceClassId, QString("Digital output %1").arg(circuit), circuit);
                ParamList parameters;
                parameters.append(Param(digitalOutputDigitalOutputNumberParamTypeId, circuit));
                descriptor.setParams(parameters);
                deviceDescriptors.append(descriptor);
            }
            emit devicesDiscovered(deviceClassId, deviceDescriptors);
            return DeviceManager::DeviceErrorAsync;
        }

        if (deviceClassId == digitalInputDeviceClassId) {
            // Create the list of available digital inputs
            QList<DeviceDescriptor> deviceDescriptors;
            for (int i = 0; i < m_digitalInputs.count(); i++) {
                const QString circuit = m_digitalInputs.at(i);

                // Offer only digital inputs which arn't in use already
                if (m_usedDigitalInputs.contains(circuit)){
                    continue;
                }
                DeviceDescriptor descriptor(deviceClassId, QString("Digital input %1").arg(circuit), circuit);
                ParamList parameters;
                parameters.append(Param(digitalInputDigitalInputNumberParamTypeId, circuit));
                descriptor.setParams(parameters);
                deviceDescriptors.append(descriptor);
            }
            emit devicesDiscovered(deviceClassId, deviceDescriptors);
            return DeviceManager::DeviceErrorAsync;
        }

        if (deviceClassId == analogInputDeviceClassId) {
            // Create the list of available digital inputs
            QList<DeviceDescriptor> deviceDescriptors;
            for (int i = 0; i < m_analogInputs.count(); i++) {
                const QString circuit = m_analogInputs.at(i);

                // Offer only analog inputs which arn't in use already
                if (m_usedAnalogInputs.contains(circuit)){
                    continue;
                }
                DeviceDescriptor descriptor(deviceClassId, QString("Analog input %1").arg(circuit), circuit);
                ParamList parameters;
                parameters.append(Param(analogInputAnalogInputNumberParamTypeId, circuit));
                descriptor.setParams(parameters);
                deviceDescriptors.append(descriptor);
            }
            emit devicesDiscovered(deviceClassId, deviceDescriptors);
            return DeviceManager::DeviceErrorAsync;
        }

        if (deviceClassId == analogOutputDeviceClassId) {
            // Create the list of available digital inputs
            QList<DeviceDescriptor> deviceDescriptors;
            for (int i = 0; i < m_analogOutputs.count(); i++) {
                const QString circuit = m_analogOutputs.at(i);

                // Offer only digital inputs which arn't in use already
                if (m_usedAnalogOutputs.contains(circuit)){
                    continue;
                }
                DeviceDescriptor descriptor(deviceClassId, QString("Analog Output %1").arg(circuit), circuit);
                ParamList parameters;
                parameters.append(Param(analogOutputAnalogOutputNumberParamTypeId, circuit));
                descriptor.setParams(parameters);
                deviceDescriptors.append(descriptor);
            }
            emit devicesDiscovered(deviceClassId, deviceDescriptors);
            return DeviceManager::DeviceErrorAsync;
        }

        if (deviceClassId == shutterDeviceClassId) {
            // Create the list of available gpios
            QList<DeviceDescriptor> deviceDescriptors;

            for (int i = 0; i < (m_relais.count()-1); i++) {
                const QString circuit = m_relais.at(i);

                // Offer only gpios which aren't in use already
                if (m_usedRelais.contains(circuit)){
                    continue;
                }

                DeviceDescriptor descriptor(deviceClassId, "Roller shutter", QString("Opening relay %1 | Closing relay %2").arg(circuit, m_relais.at(i+1)));
                ParamList parameters;
                parameters.append(Param(shutterOutputOpenParamTypeId, circuit));
                for (int a = i; a < (m_relais.count()); a++) {

                    const QString circuit = m_relais.at(a);
                    // Offer only gpios which aren't in use already
                    if (m_usedRelais.contains(circuit)){
                        continue;
                    }
                    parameters.append(Param(shutterOutputCloseParamTypeId, m_relais.at(a)));
                }
                parameters.append(Param(shutterOutputTypeOpenParamTypeId, GPIOType::relay));
                parameters.append(Param(shutterOutputTypeCloseParamTypeId, GPIOType::relay));
                descriptor.setParams(parameters);
                deviceDescriptors.append(descriptor);
            }
            emit devicesDiscovered(deviceClassId, deviceDescriptors);
            return DeviceManager::DeviceErrorAsync;
        }

        if (deviceClassId == lightDeviceClassId) {
            // Create the list of available gpios
            QList<DeviceDescriptor> deviceDescriptors;
            for (int i = 0; i < m_relais.count(); i++) {
                const QString circuit = m_relais.at(i);

                // Offer only gpios which arn't in use already
                if (m_usedRelais.contains(circuit)){
                    continue;
                }
                DeviceDescriptor descriptor(deviceClassId, QString("Relay %1").arg(circuit), circuit);
                ParamList parameters;
                parameters.append(Param(lightOutputParamTypeId, circuit));
                parameters.append(Param(lightOutputTypeParamTypeId, GPIOType::relay));
                descriptor.setParams(parameters);
                deviceDescriptors.append(descriptor);
            }

            for (int i = 0; i < m_digitalOutputs.count(); i++) {
                const QString circuit =  m_digitalOutputs.at(i);

                // Offer only gpios which arn't in use already
                if (m_usedDigitalOutputs.contains(circuit)){
                    continue;
                }
                DeviceDescriptor descriptor(deviceClassId, QString("Digital output %1").arg(circuit), circuit);
                ParamList parameters;
                parameters.append(Param(lightOutputParamTypeId, circuit));
                parameters.append(Param(lightOutputTypeParamTypeId, GPIOType::digitalOutput));
                descriptor.setParams(parameters);
                deviceDescriptors.append(descriptor);
            }
            emit devicesDiscovered(deviceClassId, deviceDescriptors);
            return DeviceManager::DeviceErrorAsync;
        }
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}

void DevicePluginUniPi::setOutput(const QString &circuit, const QString &type, bool value)
{
    QJsonObject json;
    json["cmd"] = "set";
    json["dev"] = type;
    json["circuit"] = circuit;
    json["value"] = value;

    QJsonDocument doc(json);
    QByteArray bytes = doc.toJson(QJsonDocument::Compact);
    qCDebug(dcUniPi()) << "Send command" << bytes;
    m_webSocket->sendBinaryMessage(bytes);
}

void DevicePluginUniPi::postSetupDevice(Device *device)
{
    Q_UNUSED(device);
}

void DevicePluginUniPi::deviceRemoved(Device *device)
{
    if(device->deviceClassId() == relayOutputDeviceClassId) {
        m_usedRelais.remove(device->paramValue(relayOutputRelayNumberParamTypeId).toString());
    } else if(device->deviceClassId() == digitalOutputDeviceClassId) {
        m_usedDigitalOutputs.remove(device->paramValue(digitalOutputDigitalOutputNumberParamTypeId).toString());
    } else if(device->deviceClassId() == digitalInputDeviceClassId) {
        m_usedDigitalOutputs.remove(device->paramValue(digitalInputDigitalInputNumberParamTypeId).toString());
    } else if (device->deviceClassId() == analogOutputDeviceClassId) {
        m_usedDigitalOutputs.remove(device->paramValue(analogOutputAnalogOutputNumberParamTypeId).toString());
    } else if (device->deviceClassId() == analogInputDeviceClassId) {
        m_usedDigitalOutputs.remove(device->paramValue(analogInputAnalogInputNumberParamTypeId).toString());

    } else if (device->deviceClassId() == shutterDeviceClassId) {
        if (device->paramValue(shutterOutputTypeOpenParamTypeId) == GPIOType::relay) {
            m_usedRelais.remove(device->paramValue(shutterOutputOpenParamTypeId).toString());
        } else if (device->paramValue(shutterOutputOpenParamTypeId) == GPIOType::digitalOutput) {
            m_usedDigitalOutputs.remove(device->paramValue(shutterOutputOpenParamTypeId).toString());
        }
        if (device->paramValue(shutterOutputTypeCloseParamTypeId) == GPIOType::relay) {
            m_usedRelais.remove(device->paramValue(shutterOutputCloseParamTypeId).toString());
        } else if (device->paramValue(shutterOutputOpenParamTypeId) == GPIOType::digitalOutput) {
            m_usedDigitalOutputs.remove(device->paramValue(shutterOutputCloseParamTypeId).toString());
        }

    } else if (device->deviceClassId() == lightDeviceClassId) {
        if (device->paramValue(lightOutputTypeParamTypeId) == GPIOType::relay) {
            m_usedRelais.remove(device->paramValue(lightOutputParamTypeId).toString());
        } else if (device->paramValue(lightOutputParamTypeId) == GPIOType::digitalOutput) {
            m_usedDigitalOutputs.remove(device->paramValue(lightOutputParamTypeId).toString());
        }
    }

    if (myDevices().isEmpty()) {
        m_webSocket->close();
        m_webSocket->deleteLater();
    }
}

DeviceManager::DeviceError DevicePluginUniPi::executeAction(Device *device, const Action &action)
{

    if (device->deviceClassId() == relayOutputDeviceClassId) {

        if (action.actionTypeId() == relayOutputRelayStatusActionTypeId) {
            QString relayNumber = device->paramValue(relayOutputRelayNumberParamTypeId).toString();
            int stateValue = action.param(relayOutputRelayStatusActionParamTypeId).value().toInt();

            QJsonObject json;
            json["cmd"] = "set";
            json["dev"] = "relay";
            json["circuit"] = relayNumber;
            json["value"] = stateValue;

            QJsonDocument doc(json);
            QByteArray bytes = doc.toJson(QJsonDocument::Compact);
            qCDebug(dcUniPi()) << "Send command" << bytes;
            m_webSocket->sendBinaryMessage(bytes);
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }


    if (device->deviceClassId() == digitalOutputDeviceClassId) {
        if (action.actionTypeId() == digitalOutputDigitalOutputStatusActionTypeId) {
            int digitalOutputNumber = device->paramValue(digitalOutputDigitalOutputNumberParamTypeId).toInt();
            int stateValue = action.param(digitalOutputDigitalOutputStatusActionParamTypeId).value().toInt();

            QJsonObject json;
            json["cmd"] = "set";
            json["dev"] = "do";
            json["circuit"] = digitalOutputNumber;
            json["value"] = stateValue;

            QJsonDocument doc(json);
            QByteArray bytes = doc.toJson();
            m_webSocket->sendTextMessage(bytes);
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    if (device->deviceClassId() == analogOutputDeviceClassId) {

        if (action.actionTypeId() == analogOutputAnalogOutputValueActionTypeId) {
            int analogOutputNumber = device->paramValue(analogOutputAnalogOutputNumberParamTypeId).toInt();
            double analogValue = action.param(analogOutputAnalogOutputValueActionParamTypeId).value().toDouble();

            QJsonObject json;
            json["cmd"] = "set";
            json["dev"] = "ao";
            json["circuit"] = analogOutputNumber;
            json["value"] = analogValue;

            QJsonDocument doc(json);
            QByteArray bytes = doc.toJson();
            m_webSocket->sendTextMessage(bytes);
            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }

    if (device->deviceClassId() == shutterDeviceClassId) {
        QString circuitOpen = device->paramValue(shutterOutputOpenParamTypeId).toString();
        QString typeOpen = device->paramValue(shutterOutputTypeOpenParamTypeId).toString();

        QString circuitClose = device->paramValue(shutterOutputCloseParamTypeId).toString();
        QString typeClose = device->paramValue(shutterOutputTypeCloseParamTypeId).toString();

        if (action.actionTypeId() == shutterCloseActionTypeId) {

            setOutput(circuitOpen, typeOpen, false);
            setOutput(circuitClose, typeClose, true);
            return DeviceManager::DeviceErrorNoError;
        }
        if (action.actionTypeId() == shutterOpenActionTypeId) {

            setOutput(circuitClose, typeClose, false);
            setOutput(circuitOpen, typeOpen, true);
            return DeviceManager::DeviceErrorNoError;
        }
        if (action.actionTypeId() == shutterStopActionTypeId) {
            setOutput(circuitOpen, typeOpen, false);
            setOutput(circuitClose, typeClose, false);

            return DeviceManager::DeviceErrorNoError;
        }
        return DeviceManager::DeviceErrorActionTypeNotFound;
    }
    return DeviceManager::DeviceErrorDeviceClassNotFound;
}


void DevicePluginUniPi::onWebSocketConnected()
{
    qCDebug(dcUniPi()) << "WebSocket connected";

    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &DevicePluginUniPi::onWebSocketTextMessageReceived);

    QJsonObject json;
    json["cmd"] = "all";

    QJsonDocument doc(json);
    QByteArray bytes = doc.toJson();
    m_webSocket->sendTextMessage(bytes);
}

void DevicePluginUniPi::onWebSocketDisconnected()
{
    qCDebug(dcUniPi())  << "WebSocket disconnected";

}


void DevicePluginUniPi::onWebSocketTextMessageReceived(QString message)
{
    QJsonArray array;
    QJsonObject obj;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

    // check validity of the document
    if(!doc.isNull()) {
        if(doc.isObject()) {
            obj = doc.object();
        } else if (doc.isArray()){
            array = doc.array();
        }else {
            qCDebug(dcUniPi()) << "Document is not an object nor an array";
        }
    } else {
        qCDebug(dcUniPi()) << "Invalid JSON";
    }

    for (int levelIndex = 0; levelIndex < array.size(); ++levelIndex) {
        obj = array[levelIndex].toObject();

        if (obj["cmd"] == "all") {
            //read model number
            qCDebug(dcUniPi()) << message;
        }

        if (obj["dev"] == "relay") {
            qCDebug(dcUniPi()) << "Relay:" << obj["dev"].toString() << "Circuit:" <<  obj["circuit"].toString() << "Value:" << obj["value"].toInt();

            QString circuit = obj["circuit"].toString();
            bool value = QVariant(obj["value"].toInt()).toBool();

            if (!m_relais.contains(circuit)) {
                //New Device detected
                m_relais.append(circuit);
            } else {

                foreach (Device *device, myDevices()) {
                    if (device->deviceClassId() == relayOutputDeviceClassId) {
                        if (circuit == device->paramValue(relayOutputRelayNumberParamTypeId).toString()) {
                            device->setStateValue(relayOutputRelayStatusStateTypeId, value);
                            break;
                        }
                    } else if (device->deviceClassId() == shutterDeviceClassId) {
                        if (circuit == device->paramValue(shutterOutputOpenParamTypeId).toString()) {
                            if (value && device->stateValue(shutterStatusStateTypeId).toString().contains("stop")) {
                                device->setStateValue(shutterStatusStateTypeId, "open");
                            } else if (!value && device->stateValue(shutterStatusStateTypeId).toString().contains("open")) {
                                device->setStateValue(shutterStatusStateTypeId, "stop");
                            } else {
                                qWarning(dcUniPi()) << "shutter" << device << "Output open:" << value << "Status: " << device->stateValue(shutterStatusStateTypeId).toString();
                            }

                            break;
                        }
                        if (circuit == device->paramValue(shutterOutputCloseParamTypeId).toString()) {
                            if (value && device->stateValue(shutterStatusStateTypeId).toString().contains("stop")) {
                                device->setStateValue(shutterStatusStateTypeId, "close");
                            } else if (!value && device->stateValue(shutterStatusStateTypeId).toString().contains("close")) {
                                device->setStateValue(shutterStatusStateTypeId, "stop");
                            } else {
                                qWarning(dcUniPi()) << "shutter" << device << "Output close:" << value << "Status: " << device->stateValue(shutterStatusStateTypeId).toString();
                            }
                            break;
                        }

                    } else if (device->deviceClassId() == lightDeviceClassId) {
                        if (circuit == device->paramValue(lightOutputParamTypeId).toString()) {
                            device->setStateValue(lightPowerStateTypeId, value);
                            break;
                        }
                    }
                }
            }
        }

        if (obj["dev"] == "input") {
            qCDebug(dcUniPi()) << "Input:" << obj["dev"].toString() << "Circuit:" <<  obj["circuit"].toString() << "Value:" << obj["value"].toInt();

            if (!m_digitalInputs.contains(obj["circuit"].toString())){
                //New Device detected
                m_digitalInputs.append(obj["circuit"].toString());
            } else {

                foreach (Device *device, myDevices()) {
                    if (device->deviceClassId() == digitalInputDeviceClassId) {
                        if (obj["circuit"] == device->paramValue(digitalInputDigitalInputNumberParamTypeId).toString()) {
                            device->setStateValue(digitalInputDigitalInputStatusStateTypeId, QVariant(obj["value"].toInt()).toBool());
                            break;
                        }
                    }
                }
            }
        }

        if (obj["dev"] == "output") {
            qCDebug(dcUniPi()) << "Output:" << obj["dev"].toString() << "Circuit:" <<  obj["circuit"].toString() << "Value:" << obj["value"].toInt();

            if (!m_digitalOutputs.contains(obj["circuit"].toString())){
                //New Device detected
                m_digitalOutputs.append(obj["circuit"].toString());
            } else {

                foreach (Device *device, myDevices()) {
                    if (device->deviceClassId() == digitalOutputDeviceClassId) {

                        if (obj["circuit"] == device->paramValue(digitalOutputDigitalOutputNumberParamTypeId).toString()) {
                            device->setStateValue(digitalOutputDigitalOutputStatusStateTypeId, QVariant(obj["value"].toInt()).toBool());
                            break;
                        }
                    }
                }
            }
        }

        if (obj["dev"] == "ao") {
            qCDebug(dcUniPi()) << "Analog Output:" << obj["dev"] << "Circuit:" <<  obj["circuit"].toString() << "Value:" << obj["value"].toDouble();

            if (!m_analogOutputs.contains(obj["circuit"].toString())){
                //New Device detected
                m_analogOutputs.append(obj["circuit"].toString());

            } else {
                foreach (Device *device, myDevices()) {
                    if (device->deviceClassId() == analogOutputDeviceClassId) {
                        if (obj["circuit"] == device->paramValue(analogOutputAnalogOutputNumberParamTypeId).toString()) {
                            device->setStateValue(analogOutputAnalogOutputValueStateTypeId, obj["value"].toDouble());
                            break;
                        }
                    }
                }
            }
        }

        if (obj["dev"] == "ai") {
            qCDebug(dcUniPi()) << "Analog Input:" << obj["dev"] << "Circuit:" <<  obj["circuit"].toString() << "Value:" << obj["value"].toDouble();

            if (!m_analogInputs.contains(obj["circuit"].toString())){
                //New Device detected
                m_analogInputs.append(obj["circuit"].toString());
            } else {
                foreach (Device *device, myDevices()) {
                    if (device->deviceClassId() == analogInputDeviceClassId) {

                        if (obj["circuit"] == device->paramValue(analogInputAnalogInputNumberParamTypeId).toString()) {
                            device->setStateValue(analogInputAnalogInputValueStateTypeId, obj["value"].toDouble());
                            break;
                        }
                    }
                }
            }
        }
    }
}
