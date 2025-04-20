#include <Arduino.h>
#include "clock.h"
#include "DeviceStorage.h"
#include "BluetoothMgrBle.h"

Clock rtcClock;
DeviceStorage deviceStorage;
BluetoothMgrBle bluetoothBle;

const int relayPin = 27;
const int ledPin = 2;

void onAlarm(uint8_t hour, uint8_t minute) {
	Serial.println("alerm activated");
	digitalWrite(relayPin, HIGH);
	digitalWrite(ledPin, HIGH);
}


void processCommand(String command) {
  command.trim();  // Remove leading/trailing whitespace
  command = command.substring(0, command.indexOf('\n')); // Get first line only

  if (command.startsWith("help")) {
    String rtc_status = "RTC: " + rtcClock.getRTCStatus() + "\n\n";
    bluetoothBle.sendData(rtc_status);
    String s = "getTime/N -> send current time N times\n";
    s += "setTime/HH:MM:SS -> to set time\n";
    s += "getAlarm -> get current alarm\n";
    s += "setAlarm/[HH:MM:SS,duration,...] -> set alarms\n";
    s += "test/n -> GPIO 2 & 27 ON for n sec\n";
    s += "reboot -> restart ESP32\n";
    bluetoothBle.sendData(s);
  }

  else if (command.startsWith("getTime/") || command.startsWith("gettime/")) {
    String numStr = command.substring(command.indexOf('/') + 1);
    int count = numStr.toInt();
    if (count <= 0) count = 1;  // default to 1 if invalid
    for (int i = 0; i < count; i++) {
      bluetoothBle.sendData(rtcClock.getTime() + "\n");
      delay(1000);
    }
  }

  else if (command.startsWith("setTime/")) {
    String timePart = command.substring(8); // HH:MM:SS
    int hh, mm, ss;
    if (sscanf(timePart.c_str(), "%d:%d:%d", &hh, &mm, &ss) == 3 &&
        hh >= 0 && hh < 24 && mm >= 0 && mm < 60 && ss >= 0 && ss < 60) {
      rtcClock.setTime(hh, mm, ss);
      bluetoothBle.sendData("time set");
    } else {
      bluetoothBle.sendData("invalid time format");
    }
  }

  else if (command.startsWith("getAlarm")) {
    String timing = deviceStorage.get("timing");
    if (timing.length() == 0) {
      bluetoothBle.sendData("alarm is not set yet");
    } else {
      bluetoothBle.sendData("current alarm: " + timing);
    }
  }

  else if (command.startsWith("setAlarm/")) {
    String timing = command.substring(9); // everything after "setAlarm/"
    if (timing.length() > 0) {
      deviceStorage.set("timing", timing);
      bluetoothBle.sendData("alarm set, rebooting...");
      delay(500); // small delay to allow data to send
      ESP.restart(); // reboot esp32
    } else {
      bluetoothBle.sendData("invalid alarm data");
    }
  }

  else if (command.startsWith("test/")) {
    int seconds = command.substring(5).toInt();
    if (seconds <= 0) {
      bluetoothBle.sendData("invalid duration");
    } else {
      pinMode(2, OUTPUT);
      pinMode(27, OUTPUT);
      digitalWrite(2, HIGH);
      digitalWrite(27, HIGH);
      bluetoothBle.sendData("GPIO 2 & 27 ON");
      delay(seconds * 1000);
      digitalWrite(2, LOW);
      digitalWrite(27, LOW);
      bluetoothBle.sendData("GPIO 2 & 27 OFF");
    }
  }

  else if (command == "reboot") {
    bluetoothBle.sendData("rebooting...");
    delay(500);
    ESP.restart();
  }

  else {
    bluetoothBle.sendData("unknown command");
  }

  bluetoothBle.sendData("\n--- done ---");
}

void setup() {
	Serial.begin(115200);
	deviceStorage.begin();
  pinMode(ledPin, OUTPUT);
	bluetoothBle.init("school_bell");
	bluetoothBle.setCommandCallback(&processCommand);
	
	String timing = 	deviceStorage.get("timing");
	if (timing == NULL) {
		Serial.println("timing is null, setting default");
		String timings = "['8:53:0,5','8:53:10,5','8:53:20,5','8:53:30,5','8:53:40,5','8:53:50,5','8:54:0,5','8:54:10,5','8:54:20,5','8:54:30,5','8:54:40,5','8:54:50,5','8:55:0,5','8:55:10,5','8:55:20,5','8:55:30,5','8:55:40,5','8:55:50,5','8:56:0,5','8:56:10,5','8:56:20,5','8:56:30,5','8:56:40,5','8:56:50,5','8:57:0,5','8:57:10,5','8:57:20,5','8:57:30,5','8:57:40,5','8:57:50,5','8:58:0,5','8:58:10,5','8:58:20,5','8:58:30,5','8:58:40,5','8:58:50,5','8:59:0,5','8:59:10,5','8:59:20,5','8:59:30,5','8:59:40,5','8:59:50,5','9:0:0,5','9:0:10,5','9:0:20,5','9:0:30,5','9:0:40,5','9:0:50,5','9:1:0,5','9:1:10,5','9:1:20,5','9:1:30,5','9:1:40,5','9:1:50,5','9:2:0,5','9:2:10,5','9:2:20,5','9:2:30,5','9:2:40,5','9:2:50,5','9:3:0,5','9:3:10,5','9:3:20,5','9:3:30,5','9:3:40,5','9:3:50,5','9:4:0,5','9:4:10,5','9:4:20,5','9:4:30,5','9:4:40,5','9:4:50,5','9:5:0,5','9:5:10,5','9:5:20,5','9:5:30,5','9:5:40,5','9:5:50,5','9:6:0,5','9:6:10,5','9:6:20,5','9:6:30,5','9:6:40,5','9:6:50,5','9:7:0,5','9:7:10,5','9:7:20,5','9:7:30,5','9:7:40,5','9:7:50,5','9:8:0,5','9:8:10,5','9:8:20,5','9:8:30,5','9:8:40,5','9:8:50,5','9:9:0,5','9:9:10,5','9:9:20,5','9:9:30,5','9:9:40,5','9:9:50,5','9:10:0,5','9:10:10,5','9:10:20,5','9:10:30,5','9:10:40,5','9:10:50,5','9:11:0,5','9:11:10,5','9:11:20,5','9:11:30,5','9:11:40,5','9:11:50,5','9:12:0,5','9:12:10,5','9:12:20,5','9:12:30,5','9:12:40,5','9:12:50,5','9:13:0,5','9:13:10,5','9:13:20,5','9:13:30,5','9:13:40,5','9:13:50,5','9:14:0,5','9:14:10,5','9:14:20,5','9:14:30,5','9:14:40,5','9:14:50,5','9:15:0,5','9:15:10,5','9:15:20,5','9:15:30,5','9:15:40,5','9:15:50,5','9:16:0,5','9:16:10,5','9:16:20,5','9:16:30,5','9:16:40,5','9:16:50,5','9:17:0,5','9:17:10,5','9:17:20,5','9:17:30,5','9:17:40,5','9:17:50,5','9:18:0,5','9:18:10,5','9:18:20,5','9:18:30,5','9:18:40,5','9:18:50,5','9:19:0,5','9:19:10,5','9:19:20,5','9:19:30,5','9:19:40,5','9:19:50,5','9:20:0,5','9:20:10,5','9:20:20,5','9:20:30,5','9:20:40,5','9:20:50,5','9:21:0,5','9:21:10,5','9:21:20,5','9:21:30,5','9:21:40,5','9:21:50,5','9:22:0,5','9:22:10,5','9:22:20,5','9:22:30,5','9:22:40,5','9:22:50,5','9:23:0,5','9:23:10,5','9:23:20,5','9:23:30,5','9:23:40,5','9:23:50,5','9:24:0,5','9:24:10,5','9:24:20,5','9:24:30,5','9:24:40,5','9:24:50,5','9:25:0,5','9:25:10,5','9:25:20,5','9:25:30,5','9:25:40,5','9:25:50,5','9:26:0,5','9:26:10,5','9:26:20,5','9:26:30,5','9:26:40,5','9:26:50,5','9:27:0,5','9:27:10,5','9:27:20,5','9:27:30,5','9:27:40,5','9:27:50,5','9:28:0,5','9:28:10,5','9:28:20,5','9:28:30,5','9:28:40,5','9:28:50,5','9:29:0,5','9:29:10,5','9:29:20,5','9:29:30,5','9:29:40,5','9:29:50,5','9:30:0,5','9:30:10,5','9:30:20,5','9:30:30,5','9:30:40,5','9:30:50,5','9:31:0,5','9:31:10,5','9:31:20,5','9:31:30,5','9:31:40,5','9:31:50,5','9:32:0,5','9:32:10,5','9:32:20,5','9:32:30,5','9:32:40,5','9:32:50,5','9:33:0,5','9:33:10,5','9:33:20,5','9:33:30,5','9:33:40,5','9:33:50,5','9:34:0,5','9:34:10,5','9:34:20,5','9:34:30,5','9:34:40,5','9:34:50,5','9:35:0,5','9:35:10,5','9:35:20,5','9:35:30,5','9:35:40,5','9:35:50,5','9:36:0,5','9:36:10,5','9:36:20,5','9:36:30,5','9:36:40,5','9:36:50,5','9:37:0,5','9:37:10,5','9:37:20,5','9:37:30,5','9:37:40,5','9:37:50,5','9:38:0,5','9:38:10,5','9:38:20,5','9:38:30,5','9:38:40,5','9:38:50,5','9:39:0,5','9:39:10,5','9:39:20,5','9:39:30,5','9:39:40,5','9:39:50,5','9:40:0,5','9:40:10,5','9:40:20,5','9:40:30,5','9:40:40,5','9:40:50,5','9:41:0,5','9:41:10,5','9:41:20,5','9:41:30,5','9:41:40,5','9:41:50,5','9:42:0,5','9:42:10,5','9:42:20,5','9:42:30,5','9:42:40,5','9:42:50,5','9:43:0,5','9:43:10,5','9:43:20,5','9:43:30,5','9:43:40,5','9:43:50,5','9:44:0,5','9:44:10,5','9:44:20,5','9:44:30,5','9:44:40,5','9:44:50,5','9:45:0,5','9:45:10,5','9:45:20,5','9:45:30,5','9:45:40,5','9:45:50,5','9:46:0,5','9:46:10,5','9:46:20,5','9:46:30,5','9:46:40,5','9:46:50,5','9:47:0,5','9:47:10,5','9:47:20,5','9:47:30,5','9:47:40,5','9:47:50,5','9:48:0,5','9:48:10,5','9:48:20,5','9:48:30,5','9:48:40,5','9:48:50,5','9:49:0,5','9:49:10,5','9:49:20,5','9:49:30,5','9:49:40,5','9:49:50,5','9:50:0,5','9:50:10,5','9:50:20,5','9:50:30,5','9:50:40,5','9:50:50,5','9:51:0,5','9:51:10,5','9:51:20,5','9:51:30,5','9:51:40,5','9:51:50,5','9:52:0,5','9:52:10,5','9:52:20,5','9:52:30,5','9:52:40,5','9:52:50,5']";
		timing = timings;
	} else {
		Serial.println ( " timing information read from memory,:" + timing);
	}

	rtcClock.init(timing, onAlarm);
}

void loop() {}