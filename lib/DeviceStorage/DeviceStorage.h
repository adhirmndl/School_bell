// DeviceStorage.h
#ifndef DEVICESTORAGE_H
#define DEVICESTORAGE_H

#include <Preferences.h>
#include <Arduino.h>

class DeviceStorage {
public:
    DeviceStorage(const String &namespaceName = "device");
    ~DeviceStorage();
    
    void begin();
    void reset();
    void end();
    
    String get(const String &key);

    void set(const String &key, const String &value);
    
private:
    Preferences preferences;
    String namespaceName;
};

#endif // DEVICESTORAGE_H
