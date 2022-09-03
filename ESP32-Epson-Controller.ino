#define RXD2 14
#define TXD2 13
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "private.h"

#include <SoftwareSerial.h>
SoftwareSerial swSer(RXD2,TXD2);

const char* ssid = ssid_name;
const char* password = ssid_pass;

void setup() {
  // Note the format for setting a serial port is as follows: Serial2.begin(baud-rate, protocol, RX pin, TX pin);
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  //Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  //Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  swSer.begin(9600);
  delay(1000);
  // Serial.println("Serial Txd is on pin: " + String(TX));
  // Serial.println("Serial Rxd is on pin: " + String(RX));
  if(swSer) {
    Serial.println("Success");
  } else {
    Serial.println("Error, swSer did not connect, rebooting in 10 seconds");
    delay(10000);
    ESP.restart();    
  }

    // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  ArduinoOTA.handle();
  if (Serial.available()) { // if there is data comming
    String command = Serial.readStringUntil('\n'); // read string until newline character
    sendCommand(command);
  }
}

void write(String cmd)
{
  swSer.print(cmd);
  swSer.print("\r");
  delay(100);
}

String read()
{
  String result = "";
  while (swSer.available() > 0)
  {
    result.concat((char) swSer.read());
  }
  delay(100);
  return result;
}

String sendCommand(String cmd)
{
  write(cmd);
  String result = read();
  Serial.print("read: >>");
  Serial.print(result);
  Serial.println("<<");
  result.trim();
  if (result.endsWith(":"))
    result.remove(result.length() - 1, 1);
  result.trim();
  return result;
}