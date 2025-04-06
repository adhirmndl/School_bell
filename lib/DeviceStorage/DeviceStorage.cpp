#include "DeviceStorage.h"

DeviceStorage::DeviceStorage(const String &namespaceName) : namespaceName(namespaceName) {}

DeviceStorage::~DeviceStorage() {
    end();
}

void DeviceStorage::begin() {
    Serial.println("DeviceStorage initialized");
    preferences.begin(namespaceName.c_str(), false);
}

void DeviceStorage::end() {
    Serial.println("DeviceStorage closed");
    preferences.end();
}

String DeviceStorage::get(const String &key) {
    if (!preferences.isKey(key.c_str())) {
        Serial.println("Error: Key '" + key + "' does not exist");
        return String(); // Return empty String if key does not exist
    }
    return preferences.getString(key.c_str());
}

void DeviceStorage::set(const String &key, const String &value) {
    preferences.putString(key.c_str(), value);
}

void DeviceStorage::reset() {
    preferences.clear();
}
