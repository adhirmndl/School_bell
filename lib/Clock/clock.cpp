#include "clock.h"
#include <RTClib.h>
#include <Wire.h>

RTC_DS3231 rtc;
Clock* staticClockInstance = nullptr;

Clock::Clock() : lastCheckedDay(0) {
    staticClockInstance = this;
}

void Clock::init(const String& alarmTimings, std::function<void(uint8_t, uint8_t)> callback) {
    Wire.begin(21, 22);
    if (!rtc.begin()) {
        Serial.println("Couldn't find RTC");
        while (1);
    }

    if (rtc.lostPower()) {
        Serial.println("RTC lost power, setting default time!");
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    alarmCallback = callback;
    parseAlarms(alarmTimings);

    xTaskCreatePinnedToCore(
        alarmTask,
        "AlarmTask",
        4096,
        this,
        1,
        nullptr,
        1
    );
}

void Clock::setTime(uint8_t hour, uint8_t minute, uint8_t second) {
    DateTime now = rtc.now();
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, second));
}

String Clock::getTime() {
    DateTime now = rtc.now();
    char buffer[9];
    snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", now.hour(), now.minute(), now.second());
    return String(buffer);
}

void Clock::resetDailyTriggers() {
    for (Alarm& alarm : alarms) {
        alarm.triggeredToday = false;
    }
}

void Clock::parseAlarms(const String& timingStr) {
    alarms.clear();

    String cleanStr = timingStr;
    cleanStr.replace("[", "");
    cleanStr.replace("]", "");
    cleanStr.replace("'", "");
    cleanStr.replace(" ", "");

    int start = 0;
    while (start < cleanStr.length()) {
        int comma = cleanStr.indexOf(',', start);
        if (comma == -1) break;

        String timePart = cleanStr.substring(start, comma);
        int nextComma = cleanStr.indexOf(',', comma + 1);
        if (nextComma == -1) nextComma = cleanStr.length();

        String durationPart = cleanStr.substring(comma + 1, nextComma);

        int firstColon = timePart.indexOf(':');
        int secondColon = timePart.indexOf(':', firstColon + 1);
        if (firstColon == -1 || secondColon == -1) break;

        uint8_t hour = timePart.substring(0, firstColon).toInt();
        uint8_t minute = timePart.substring(firstColon + 1, secondColon).toInt();
        uint8_t second = timePart.substring(secondColon + 1).toInt();
        uint8_t duration = durationPart.toInt();

        alarms.emplace_back(hour, minute, second, duration);

        start = nextComma + 1;
    }

    Serial.println("✅ Parsed alarms:");
    for (const Alarm& alarm : alarms) {
        Serial.printf(" - %02d:%02d:%02d for %d sec\n", alarm.hour, alarm.minute, alarm.second, alarm.duration);
    }
}

void Clock::alarmTask(void* param) {
    Clock* clock = static_cast<Clock*>(param);
    while (true) {
        clock->checkAlarm();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void Clock::defuseAlarmTask(void* param) {
    Alarm* alarm = static_cast<Alarm*>(param);
    vTaskDelay(alarm->duration * 1000 / portTICK_PERIOD_MS);
    alarm->isActive = false;
    Serial.printf("✅ Alarm at %02d:%02d:%02d defused after %d sec\n", alarm->hour, alarm->minute, alarm->second, alarm->duration);
    digitalWrite(27, LOW);
    digitalWrite(2, LOW);
    vTaskDelete(nullptr);
}

void Clock::checkAlarm() {
    DateTime now = rtc.now();
    time_t currentDay = now.day();

    if (currentDay != lastCheckedDay) {
        resetDailyTriggers();
        lastCheckedDay = currentDay;
    }

    for (Alarm& alarm : alarms) {
        if (!alarm.triggeredToday &&
            now.hour() == alarm.hour &&
            now.minute() == alarm.minute &&
            now.second() == alarm.second) {

            alarm.triggeredToday = true;
            alarm.isActive = true;

            Serial.printf("🔔 Alarm triggered at %02d:%02d:%02d\n", alarm.hour, alarm.minute, alarm.second);

            if (alarmCallback) {
                alarmCallback(alarm.hour, alarm.minute);
            }

            xTaskCreatePinnedToCore(
                defuseAlarmTask,
                "DefuseAlarmTask",
                2048,
                &alarm,
                1,
                nullptr,
                1
            );
        }
    }
}