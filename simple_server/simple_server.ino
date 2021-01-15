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
// https://www.w3schools.com/tags/tag_button.asp (example using buttons, not used yet)
// https://github.com/me-no-dev/AsyncTCP
// https://github.com/me-no-dev/ESPAsyncWebServer
//
//

const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>
<button type="button" onclick="alert('Forward')">Forward</button>
<button type="button" onclick="alert('STOP')">STOP!</button>
<button type="button" onclick="alert('Backwards')">Backwards</button>
<button type="button" onclick="alert('LTurn')">LTurn</button>
<button type="button" onclick="alert('RTurn')">RTurn</button>

<center>
<h1>Buggy Controller</h1><br>
<a href="FORWARD" target="myIframe">FORWARD</a><br>
<a href="BACKWARD" target="myIframe">BACKWARDS</a><br>
<a href="RIGHT" target="myIframe">LEFT</a><br>
<a href="LEFT" target="myIframe">RIGHT</a><br>
<a href="STOP" target="myIframe">STOP</a><br>
Distance:<iframe name="myIframe" width="100" height="25" frameBorder="0"><br>
Message:<iframe name="myIframe" width="100" height="25" frameBorder="0"><br>
</body>
</html>
)=====";

// Set up web server 

// Creating a web server on port 80
AsyncWebServer server(80);

const char* ssid = "lsl001";
const char* password = "ena429okec";
const char* PARAM_MESSAGE = "message";

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

//
//
//

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

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", MAIN_page);
    });

    // Send a GET request to <IP>/get?message=<message>
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
        String message;
        if (request->hasParam(PARAM_MESSAGE)) {
            message = request->getParam(PARAM_MESSAGE)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, GET: " + message);
    });

    // Send a POST request to <IP>/post with a form field message set to <message>
    server.on("/post", HTTP_POST, [](AsyncWebServerRequest *request){
        String message;
        if (request->hasParam(PARAM_MESSAGE, true)) {
            message = request->getParam(PARAM_MESSAGE, true)->value();
        } else {
            message = "No message sent";
        }
        request->send(200, "text/plain", "Hello, POST: " + message);
    });

  // Send a GET request to <ESP_IP>/state
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(digitalRead(output)).c_str());
  });
    
    server.onNotFound(notFound);

    server.begin();
    
}

void loop() {
  
}
