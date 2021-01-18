// A simple server implementation showing how to:
//  * serve static messages
//  * read GET and POST parameters
//  * handle missing pages / 404s

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

//
// HTML code to make a simple webpage with buttons to send messages to the buggy
//
// Sources:
// https://github.com/me-no-dev/AsyncTCP
// https://github.com/me-no-dev/ESPAsyncWebServer
//
// Forms input value of command
//

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<center>
<h1>Buggy Controller</h1><br>
<b>Commands:</b><br>
<p>Input in sencond how long the command should last</p>
<p>You cannot send the same command twice in a row</p>
<form action="/get">
  Stop: <input type="text" name="stop">
  <input type="submit" value="Send">
</form><br>
<form action="/get">
  Forward: <input type="text" name="forward">
  <input type="submit" value="Send">
</form><br>
<form action="/get">
  Backward: <input type="text" name="backward">
  <input type="submit" value="Send">
</form><br>
<form action="/get">
  Left: <input type="text" name="leftTurn">
  <input type="submit" value="Send">
</form><br>
<form action="/get">
  Right: <input type="text" name="rightTurn">
  <input type="submit" value="Send">
</form><br>

<p>Distance: %PLACEHOLDER_DISTANCE%m</p>
<p>Current Action: %PLACEHOLDER_ACTION% </p>

</body>
</html>

)=====";

// Code to refresh the whole page in second intervals
//<meta http-equiv="refresh" content="3">

// Creating a web server on port 80
AsyncWebServer server(80);

// My Network Credentials
const char* ssid = "lsl001";
const char* password = "ena429okec";

// Error 404 code, when page is not found
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

// https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
// https://randomnerdtutorials.com/esp32-esp8266-input-data-html-form/
//

// Const chars used for inputs from html forms
const char* FORWARD = "forward";
const char* BACKWARD = "backward";
const char* STOP = "stop";
const char* LEFT_TURN = "leftTurn";
const char* RIGHT_TURN = "rightTurn";

// https://techtutorialsx.com/2018/07/23/esp32-arduino-http-server-template-processing-with-multiple-placeholders/
// Returns String for the palceholders in the HTML code
// Add actual Distance, and action from the arduino here when they arrive from SPI
String processor(const String& var)
{
  
  if(var == "PLACEHOLDER_DISTANCE"){
    return String(random(10, 20));
  }

  else if(var == "PLACEHOLDER_ACTION"){
    return String(random(0, 50));
  }
  
  return String();
}

void setup() {

    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.printf("WiFi Failed!\n");
        return;
    }

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Returns infomation about sensors and prints in placeholders (Distance and Action being performed)
    // Will refresh the whole page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", MAIN_page, processor);
    });

    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    String inputParam;
    // STOP button triggered
    if (request->hasParam(STOP)) {
      inputMessage = request->getParam(STOP)->value();
      inputParam = STOP;
    }
    // BACKWARD button triggered
    else if (request->hasParam(BACKWARD)) {
      inputMessage = request->getParam(BACKWARD)->value();
      inputParam = BACKWARD;
    }
    // FORWARD button triggered
    else if (request->hasParam(FORWARD)) {
      inputMessage = request->getParam(FORWARD)->value();
      inputParam = FORWARD;
    }
    // RIGHT turn triggered
    else if (request->hasParam(RIGHT_TURN)) {
      inputMessage = request->getParam(RIGHT_TURN)->value();
      inputParam = RIGHT_TURN;
    }
    // LEFT turn triggered
    else if (request->hasParam(LEFT_TURN)) {
      inputMessage = request->getParam(LEFT_TURN)->value();
      inputParam = LEFT_TURN;
    }
    // Bug loop (This should not be possible)
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    Serial.println(inputParam);
    });
     
    server.onNotFound(notFound);

    server.begin();
    
}

// Forever loop empty as wanting to be power efficient by using asyncronous commands
void loop() { }
