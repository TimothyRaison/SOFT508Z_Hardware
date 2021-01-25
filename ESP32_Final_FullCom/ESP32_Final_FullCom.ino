// This is the final combined sketch of the ESP32 code, this includes:
// Hosting a asyncronous web server
// Reciving inputs from webpage inputs
// Posting sensor data onto webpage
// Sending and reciving SPI messages in a full duplex to Arduino mega (ESP is master)

// Librarys used
#include <SPI.h> // Library for SPI communication
#include <WiFi.h> // WiFi library
#include <AsyncTCP.h> // Library for asyncronous TCP communication for ESP32 processor
#include <ESPAsyncWebServer.h> // Library for hosting a syncronous web server
#include <string.h> // Used to process strings

// Will be hosting on port 80 as this is the port for http (HyperText Transfer Protocol)
#define port 80
// Max size for message buffer
#define maxMessage 50

// Global variables that saves the actual distance/humidity/action being performed by buggy to beprinted on webpage
// Buffer and two pointers for a circular buffer(*inp pointer will chase *outp)
int bufferSize = 20;
volatile char *inp;
volatile char *outp;
char inputBuf[20];

// Variables to store message infomation recived from the arduino over SPI
uint8_t distance;
char action;
uint8_t rpm;
char message[64];

// Chars for inputs from html forms
const char* FORWARD = "f";
const char* BACKWARD = "b";
const char* STOP = "s";
const char* LEFT_TURN = "l";
const char* RIGHT_TURN = "r";

// Chars for data recived over SPI
const uint8_t DATA_1 = '1';
const uint8_t DATA_2 = '2';
const uint8_t DATA_3 = '3';

// Source for processor loop:
// https://techtutorialsx.com/2018/07/23/esp32-arduino-http-server-template-processing-with-multiple-placeholders/
// Returns String for the palceholders in the HTML code
// Add actual Distance, and action from the arduino here when they arrive from SPI
String processor(const String& var)
{

  // Getting current message over SPI
  SPIReceive();

  // Updating distance
  if (var == "PLACEHOLDER_DISTANCE") {
    return String(distance); // This needs actual Distance
  }

  // Updating Action (getting string depending on message recived)
  else if (var == "PLACEHOLDER_ACTION") {
    switch (action) {
      case 's':
        return String("Not moving");
        break;
      case 'f':
        return String("Going forward");
        break;
      case 'l':
        return String("Turning left");
        break;
      case 'r':
        return String("Turning right");
        break;
      case 'b':
        return String("Going back");
        break;
      default:
        // Not valid action
        return String("Im confused");
    }
  }

  // Updating RPM
  else if (var == "PLACEHOLDER_RPM") {
    return String(rpm);
  }
  // If var is not matching any palceholder return nothing
  return String();
}

// HTML string for web server
// %Placeholders% -> placeholder structure (where values get inputted)
// Depending on the name of the form, when the button is pressed (on that form) will send a certain action (form s, sends stop for example)
// <meta http-equiv="refresh" content="1;/" />  -> Refreshes the page every second
const char MAIN_page[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<body>

<center>
<h1>Buggy Controller</h1>
<b>Commands:</b><br>
<p>Input commands for buggy</p>
<form action="/get">
  <input type="submit" name="s" value="STOP!">
</form><br>
<form action="/get">
  Forward: <input type="text" name="f">
  <input type="submit" value="Send">
</form><br>
<form action="/get">
  Backward: <input type="text" name="b">
  <input type="submit" value="Se nd">
</form><br>
<form action="/get">
  Left: <input type="text" name="l">
  <input type="submit" value="Send">
</form><br>
<form action="/get">
  Right: <input type="text" name="r">
  <input type="submit" value="Send">
</form><br>

<p>Update every second</p>

<p>Distance: %PLACEHOLDER_DISTANCE%cm</p>
<p>RPM (Rotations per minute): %PLACEHOLDER_RPM%</p>
<p>Action (Current action being performed): %PLACEHOLDER_ACTION% </p>

<meta http-equiv="refresh" content="1;/" />

</body>
</html>
)=====";

// Creating a web server on port 80, http port
AsyncWebServer server(port);

// My Network Credentials
const char* ssid = "lsl001";
const char* password = "ena429okec";

// Full message that will be sent over SPI
char fullMessage[maxMessage];

// Error 404 code, when page is not found
void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}

// Uninitalised pointers to SPI objects
SPIClass * vspi = NULL;

// SPI clock rate
static const int spiClk = 1000000; // 1 MHz

// Setup loop
void setup() {

   // Initialise SPIClass attached to VSPI (VSPI is pre setup SPI on specified pins as ESP can only SPI over VSPI or HSPI pins)
  vspi = new SPIClass(VSPI);

  // Initialise VSPI with default pins
  // VSPI source: https://github.com/espressif/arduino-esp32/blob/master/libraries/SPI/examples/SPI_Multiple_Buses/SPI_Multiple_Buses.ino
  // SCLK = 18, MISO = 19, MOSI = 23, SS = 5
  vspi->begin();

  pinMode(5, OUTPUT); //VSPI SS pin

  // Start UART
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Prints dots to terminal untill connected to network
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Get RSSI (Received Signal Strength Indicator)
  long rssi = WiFi.RSSI();

  // Confirming connection to terminal, IP address, port, and RSSI
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");
  Serial.println("Port:");
  Serial.println(port);

  // Init both pointers to start of buffer
  inp = inputBuf;
  outp = inputBuf;

    // Send the main page if any users connect to the server
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", MAIN_page, processor);
    });

    // If the /get page is requested then find out what button triggered it and send command over SPI accordingly
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {

    // Variables to hold 
    String inputMessage;
    String inputParam;
    
    // BACKWARD button triggered
    if (request->hasParam(BACKWARD)) {
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
    // Stop loop (When /get is called and no message) 
    // If bugged or currupted message buggy will just stop
    else {
      // No input for stop as it is forever
      inputMessage = "";
      inputParam = STOP;
    }
    
    // Prints for testing
    //Serial.println(inputParam);
    //Serial.println(inputMessage);

    // Send message over SPI to the buggy
    // Building message and putting in packet buffer string
    strcpy(fullMessage,inputParam.c_str());
    strcat(fullMessage,inputMessage.c_str());
    
    // Setting the message size interger to actual length of the message
    int messageSize = strlen(fullMessage);
    
    if (messageSize < maxMessage){

  // Start SPI transaction (print for testing)
  //Serial.println("Start SPI Transaction");

  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(5, LOW); // Pull SS low to prep other end for transfer

    // For loop that filters though the message and sends the message byte by byte
    for(int p = 0;p < messageSize;p++) {

    // Converting to byte for SPI
    byte SPIdata = fullMessage[p];
    
    // Testing prints
    //Serial.print("byte: '");
    //Serial.print(SPIdata);
    //Serial.println("'");

    // Send data over VSPI
    vspi->transfer(SPIdata);

    }

      // Pull SS high to signify end of data transfer
      digitalWrite(5, HIGH);
      // End SPI Transaction
      vspi->endTransaction();
      // Test print
      //Serial.println("End SPI Transaction");

    }
    else {
      //Serial.println("error string too long");
    }
    
    // Clear buffer
    memset(fullMessage, '\0', sizeof(fullMessage));
    
    // Resetting to origional page (could have 302 return command) 
    request->send_P(200, "text/html", MAIN_page, processor); 
    
    }); // ending /get triggered function

    // Call error 404, if page not found
    server.onNotFound(notFound);

    // Start hosting webpage
    server.begin();

}

// Sending nothing over SPI (When the master sends it reciveds at the same time)
void SPIReceive() {

    // Print for testing
    //Serial.println("Receive start");

    // Start vspi SPI
    vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
    // SS low (telling other devices to get SPI ready)
    digitalWrite(5, LOW);

    // SPI work in a full duplex way, meaning I am reciving the previous request while sending a request
    // Sending request for data 1 (Distance)
    vspi->transfer(DATA_1);
    delay(2);
    
    // Sending request for data 2 (RPM), while reciving the distance
    distance = vspi->transfer(DATA_2);
    delay(2);
    
    // Sending request for data 3(Action), while reciving the RPM
    rpm = vspi->transfer(DATA_3);
    delay(2);
    
    // Sending null data so that the last bit can be recived over SPI
    action = vspi->transfer(0);
    
    // SS high
    digitalWrite(5, HIGH);
    // Stopping SPI transaction
    vspi->endTransaction();

    // Building a buffer using sprintf and dumping all the recived data in it for testing (printf)
    //sprintf(message, ">: d:%i, r:%i, a:%c", distance, rpm, action);
    //Serial.println(message);
    //Serial.println("Receive complete");
}

// Main loop (empty as everything is asyncronously triggered)
void loop() {}
