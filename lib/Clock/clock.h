#pragma once
#include <Arduino.h>
#include <vector>
#include <functional>

struct Alarm {
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t duration;
    bool triggeredToday;
    bool isActive;

    Alarm(uint8_t h, uint8_t m, uint8_t s, uint8_t d)
        : hour(h), minute(m), second(s), duration(d),
          triggeredToday(false), isActive(false) {}
};

class Clock {
public:
    Clock();
    void init(const String& alarmTimings, std::function<void(uint8_t, uint8_t)> callback);
    void setTime(uint8_t hour, uint8_t minute, uint8_t second);
    String getTime();

private:
    std::vector<Alarm> alarms;
    std::function<void(uint8_t, uint8_t)> alarmCallback;
    time_t lastCheckedDay;

    void resetDailyTriggers();
    void parseAlarms(const String& timingStr);
    void checkAlarm();

    static void alarmTask(void* param);
    static void defuseAlarmTask(void* param);
};