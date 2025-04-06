// BluetoothMgrBle.h
#ifndef BLUETOOTHMGRBLE_H
#define BLUETOOTHMGRBLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// #define SERVICE_UUID        "12345678-1234-5678-1234-56789abcdef0"
// #define CHARACTERISTIC_UUID "abcdef01-1234-5678-1234-56789abcdef0"
// #define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
// #define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // RX Characteristic
// #define TX_CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // TX Characteristic
#define BLUETOOTH_BLE_SERVICE_UUID          "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define BLUETOOTH_BLE_RX_CHARACTERISTIC_UUID "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"  // Write (App -> ESP32)
#define BLUETOOTH_BLE_TX_CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"  // Notify (ESP32 -> App)

class BluetoothMgrBle : public BLEServerCallbacks, public BLECharacteristicCallbacks {
public:
    BluetoothMgrBle();
    void init(const char* deviceName);
    void sendData(const String& data);
    void setCommandCallback(void (*callback)(String));

private:
    BLEServer* pServer;
    BLECharacteristic* pCharacteristic;
    bool deviceConnected;
    void (*commandCallback)(String);

    void onWrite(BLECharacteristic* characteristic) override;
    void onConnect(BLEServer* pServer) override;
    void onDisconnect(BLEServer* pServer) override;
};

#endif