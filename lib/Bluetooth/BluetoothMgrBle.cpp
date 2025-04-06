// BluetoothMgrBle.cpp
#include "BluetoothMgrBle.h"
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

BluetoothMgrBle::BluetoothMgrBle() : deviceConnected(false), commandCallback(nullptr) {}

void BluetoothMgrBle::init(const char* deviceName) {
    BLEDevice::init(deviceName);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(this);

    BLEService* pService = pServer->createService(BLUETOOTH_BLE_SERVICE_UUID);

    // TX Characteristic (ESP32 -> App)
    pCharacteristic = pService->createCharacteristic(
        BLUETOOTH_BLE_TX_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pCharacteristic->addDescriptor(new BLE2902());

    // RX Characteristic (App -> ESP32)
    BLECharacteristic* pRxCharacteristic = pService->createCharacteristic(
        BLUETOOTH_BLE_RX_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE
    );
    pRxCharacteristic->setCallbacks(this);

    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(BLUETOOTH_BLE_SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}

void BluetoothMgrBle::sendData(const String& data) {
    if (deviceConnected) {
        pCharacteristic->setValue(data.c_str());
        pCharacteristic->notify();
    }
}

void BluetoothMgrBle::setCommandCallback(void (*callback)(String)) {
    commandCallback = callback;
}

void BluetoothMgrBle::onWrite(BLECharacteristic* characteristic) {
    std::string value = characteristic->getValue();
    if (value.length() > 0) {
        // Send response to connected device
        // sendData("Command received: " + String(value.c_str()));

        // If a command callback is set, call it with the received data
        if (commandCallback) {
            commandCallback(String(value.c_str()));
        }
    }
}

void BluetoothMgrBle::onConnect(BLEServer* pServer) {
    deviceConnected = true;
    sendData("you are connected ....");
    Serial.println("Bluetooth Ble device Connected");
}

void BluetoothMgrBle::onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Bluetooth Ble device Disconnected");
    BLEDevice::startAdvertising();
}
