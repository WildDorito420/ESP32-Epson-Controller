# ESP32 Epson Controller

Inspired by [https://github.com/norman-thomas/epson-projector-remote](https://github.com/norman-thomas/epson-projector-remote)
 
This is a small project I am taking on to turn the Espon projectors at my workplace into something a bit more smart, instead of paying and using Epson's WiFi dongles.
My original goal was to get the ESP(s) to connect to a web socket and to be controlled via that.

Some goals I thought of on the spot (can't confirm if these will ever be completed):
- RS232 passthrough from existing controller
- MQTT Support
- SNMP Support
- Web page to control on each ESP32

## Resources:
- [Epson Business Projector Guide](https://files.support.epson.com/pdf/pl600p/pl600pcm.pdf)

## Required Libraries:
- EspSoftwareSerial
- WiFi (comes with ESP32 library)
- ESPmDNS.h (comes with ESP32 library)
- WiFiUdp.h (comes with ESP32 library)
- ArduinoOTA.h (comes with ESP32 library)
