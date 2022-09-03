#ifndef HTML_H
#define HTML_H

String SendHTML(uint8_t pwrstate,uint8_t avstate,uint8_t frzstate, String msg){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>Projector Control</title>\n";
  ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr +=".button-on {background-color: #3498db;}\n";
  ptr +=".button-on:active {background-color: #2980b9;}\n";
  ptr +=".button-off {background-color: #34495e;}\n";
  ptr +=".button-off:active {background-color: #2c3e50;}\n";
  ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr +="</style>\n";
  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<h1>ESP32 Web Server</h1>\n";
  ptr +="<h3>"+ msg +"</h3>\n";
  
   if(pwrstate == 1)
  {ptr +="<p>Power Status: ON</p><a class=\"button button-off\" href=\"/power\">OFF</a>\n";}
  else
  {ptr +="<p>Power Status: OFF</p><a class=\"button button-on\" href=\"/power\">ON</a>\n";}

  if(avstate == 1)
  {ptr +="<p>A/V Mute Status: ON</p><a class=\"button button-off\" href=\"/avmute\">OFF</a>\n";}
  else
  {ptr +="<p>A/V Mute Status: OFF</p><a class=\"button button-on\" href=\"/avmute\">ON</a>\n";}

  if(frzstate == 1)
  {ptr +="<p>Freeze Status: ON</p><a class=\"button button-off\" href=\"/freeze\">OFF</a>\n";}
  else
  {ptr +="<p>Freeze Status: OFF</p><a class=\"button button-on\" href=\"/freeze\">ON</a>\n";}

  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

String redirectHome() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta http-equiv=\"refresh\" content=\"0;url=/\"></head>\n";
  ptr += "<body>Action triggered, redirecting home</body></html>";

  return ptr;
  
}

#endif