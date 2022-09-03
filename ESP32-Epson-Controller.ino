#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebServer.h>

#include "private.h"
#include "types.h"
#include "html.h"
//#include "power.h"
//#include "commands.h"

#define RXD2 14
#define TXD2 13
#include <SoftwareSerial.h>
SoftwareSerial swSer(RXD2,TXD2);

WebServer server(80);

String lastStatus = "";
int avMute(int toggle=0);
int freeze(int toggle=0);
int power(int toggle=0);
String htmlRender(String msg=lastStatus);

const char* ssid = ssid_name;
const char* password = ssid_pass;
const char* device_name = dev_name;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(device_name);
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
  ArduinoOTA.setHostname(device_name);

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
  delay(250);
  server.on("/", handle_OnConnect);
  server.on("/power", handle_power);
  server.on("/avmute", handle_avmute);
  server.on("/freeze", handle_freeze);
  server.onNotFound(handle_NotFound);
  
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  ArduinoOTA.handle();
  server.handleClient();
  if (Serial.available()) { 
    String command = Serial.readStringUntil('\n'); 
    sendCommand(command);
  }
}

String htmlRender(String msg) {
    int pwr = power();
    int av = avMute();
    int fr = freeze();
    return SendHTML(pwr, av, fr, msg);
}

void handle_OnConnect() {
  server.send(200, "text/html", htmlRender()); 
}

void handle_power() {
  String msg = "";
  int stat = power(1);
  if (stat == 0) {
    msg = "power toggle success";
  } else if (stat == -1) {
    msg = "power toggle failed";
  }
  lastStatus = msg;
  server.send(200, "text/html", redirectHome()); 
}

void handle_avmute() {
  String msg = "";
  int stat = avMute(1);
  if (stat == 0) {
    msg = "AV Mute toggle success";
  } else if (stat == -1) {
    msg = "AV Mute toggle failed";
  }
  lastStatus = msg;
  server.send(200, "text/html", redirectHome()); 
}

void handle_freeze() {
  String msg = "";
  int stat = freeze(1);
  if (stat == 0) {
    msg = "freeze toggle success";
  } else if (stat == -1) {
    msg = "freeze toggle failed";
  }
  lastStatus = msg;
  server.send(200, "text/html", redirectHome()); 
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

int checkError(String arg)
{
  const String result = sendCommand("ERR?");
  if (result.startsWith("ERR="))
    result.substring(4);
  return result.toInt();
}

int ok(String arg)
{
  return checkError(arg) == 0 ? 0 : -1;
}


bool detectError(String result, String errorMessage)
{
  const bool error = result.equals("ERR");
  if (error)
    report(errorMessage);
  return error;
}

void report(String descr)
{
  Serial.println(descr);
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
  Serial.print("Sending command: ");
  Serial.println(cmd);
  write(cmd);
  String result = read();
  result.replace(":", "");
  Serial.print("read: >>");
  Serial.print(result);
  Serial.println("<<");
  result.trim();
  if (result.endsWith(":"))
    result.remove(result.length() - 1, 1);
  result.trim();
  return result;
}

int avMute(int toggle) {
  String result = sendCommand("MUTE?");
  if (detectError(result, "projector mute check failed"))
    return -1;
  
  if(result.equals("MUTE=OFF")) {
    if(toggle == 0) {
      return 0;
    } else {
      result = sendCommand("MUTE ON");
    }
  } else if(result.equals("MUTE=ON")){
    if (toggle == 0) {
      return 1;      
    } else {
      result = sendCommand("MUTE OFF");
    }
  }
    
  if (detectError(result, "projector mute toggle failed")) {
    return -1;
  } else {
    const bool cond = result.equals("");
    cond ? report("projector mute toggle success") : report("projector mute toggle failed");
    return cond ? 0 : -1;
  }
}


int freeze(int toggle) {
  String result = sendCommand("FREEZE?");
  if (detectError(result, "projector freeze check failed"))
    return -1;
  
  if(result.equals("FREEZE=OFF")) {
    if(toggle == 0) {
      return 0;
    } else {
      result = sendCommand("FREEZE ON");
    }
  } else if(result.equals("FREEZE=ON")){
    if (toggle == 0) {
      return 1;      
    } else {
      result = sendCommand("FREEZE OFF");
    }
  }
    
  if (detectError(result, "projector freeze toggle failed")) {
    return -1;
  } else {
    const bool cond = result.equals("");
    cond ? report("projector freeze toggle success") : report("projector freeze toggle failed");
    return cond ? 0 : -1;
  }
}

int power(int toggle) {
  POWER status = getPower();
  String result = "";

  if(status == OFF || status == COOLINGDOWN || status == STANDBYNETWORKON || status == ABNORMALSTANDBY) {
    if(toggle == 0) {
      return 0;
    } else {
      result = sendCommand("PWR ON");
      delay(2500);
    }
  } else if(status == ON || status == WARMUP){
    if (toggle == 0) {
      return 1;      
    } else {
      result = sendCommand("PWR OFF");
      delay(250);
    }
  }
    
  if (detectError(result, "projector power toggle failed")) {
    return -1;
  } else {
    const bool cond = result.equals("");
    cond ? report("projector power toggle success") : report("projector power toggle failed");
    return cond ? 0 : -1;
  }
}


POWER getPower()
{
  String result = sendCommand("PWR?");
  if (detectError(result, "projector status check failed"))
    return UNKNOWN;

  if (result.equals("PWR=00"))
    return OFF;
  if (result.equals("PWR=01"))
    return ON;
  if (result.equals("PWR=02"))
    return WARMUP;
  if (result.equals("PWR=03"))
    return COOLINGDOWN;
  if (result.equals("PWR=04"))
    return STANDBYNETWORKON;
  if (result.equals("PWR=05"))
    return ABNORMALSTANDBY;

  report("unexpected status: " + result);
  return UNKNOWN;
}

// int power(String arg)
// {
//   if (arg.equals("on") || arg.equals("1"))
//     return powerOn();
//   if (arg.equals("off") || arg.equals("0"))
//     return powerOff();
//   if (arg.equals("?") || arg.equals(""))
//     return powerStatus();

//   report("unexpected arg: '" + arg + "'");
//   return -1;
// }

int powerOn()
{
  const POWER status = getPower();
  if (status == ON || status == WARMUP)
  {
      //mqttClient.publish(MQTT_PROJECTOR_STATUS, "on", false, 1);
      return 0;
  }

  const String result = sendCommand("PWR ON");
  if (detectError(result, "projector turn on failed"))
  {
    return -1;
  }

  report("PWR ON: " + result);

  const bool cond = result.equals("");
  const String status_str = cond ? "on" : "off";
  //mqttClient.publish(MQTT_PROJECTOR_STATUS, status_str, false, 1);
  cond ? report("projector turned on") : report("projector turn on failed");
  return cond ? 0 : -1;
}

int powerOff()
{
  const POWER status = getPower();
  if (status == OFF || status == STANDBYNETWORKON)
  {
    //mqttClient.publish(MQTT_PROJECTOR_STATUS, "on", false, 1);
    return 0;
  }

  const String result = sendCommand("PWR OFF");
  if (detectError(result, "projector turn off failed"))
  {
    return -1;
  }
  
  report("PWR OFF: " + result);
  
  const bool cond = result.equals("");
  String status_str = cond ? "off" : "on";
  //mqttClient.publish(MQTT_PROJECTOR_STATUS, status_str, false, 1);
  cond ? report("projector turned off") : report("projector turn off failed");
  return cond ? 0 : -1;
}

int powerStatus()
{
  const POWER status = getPower();
  if (status == UNKNOWN)
  {
    return -1;
  }
  String status_str = (status == ON || status == WARMUP) ? "on" : "off";
  //mqttClient.publish(MQTT_PROJECTOR_STATUS, status_str, false, 1);
  return (status == ON || status == WARMUP) ? 1 : 0;
}

