
// Code for ESP32 WIFI development board
// Will recive and send WiFi packages to my PC and Use SPI to talk to the Arduino Mega
// Timothy Raison

#include <SPI.h>
#include <WiFi.h> // https:www.arduino.cc/en/Reference/WiFi
#include <WiFiUdp.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <AsyncTCP.h> // https://github.com/me-no-dev/ESPAsyncWebServer
#include <ESPAsyncWebServer.h>

// Uninitalised pointers to SPI objects
SPIClass * vspi = NULL;

// My WIFI ssid and password
const char* ssid = "lsl001";
const char* password = "ena429okec";

// Setup UDP
WiFiUDP Udp;

// SPI clock rate
static const int spiClk = 1000000; // 1 MHz

// Network key Index number (needed only for WEP)
int keyIndex = 0;

// local port to listen on
unsigned int localPort = 2390;

// buffer to hold incoming packet
char packetBuffer[255];

// Test buffer so I know packet has been recieved
char ReplyBuffer[] = "acknowledged";

// Current time
unsigned long currentTime = millis();

// Previous time
unsigned long previousTime = 0;

// Define timeout time in milliseconds
const long timeoutTime = 2000;

void setup() {

  // Initialise SPIClass attached to VSPI
  vspi = new SPIClass(VSPI);

  // Initialise VSPI with default pins
  // SCLK = 18, MISO = 19, MOSI = 23, SS = 5
  vspi->begin();

  pinMode(5, OUTPUT); //VSPI SS pin

  // Start UART
  Serial.begin(115200);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Prints dots to terminal untill connected to netowrk
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
  Serial.println(localPort);

  // Starts UDP
  Udp.begin(localPort);

}

byte data; // SPI 8 bytes to be sent
int SPIReady = 0; // Bit indicating to trasfer data

// Main loop
void loop() {

  // Check if packet recived (needs ISR)
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    // Check for packets over WIFI
    packetRecived(packetSize);
  }

  // Pause for testing
  delay(500);

}

// Need a recived
void SPIRecived() {

  // what to do when SPI is recived from Arduino (Slave to master)

}

void WiFiChange() {



}

// Check if any packets have been recived (needs to be triggered by ISR)
char packetRecived(int packetSize) {

  Serial.print("Received packet of size ");
  Serial.println(packetSize);
  Serial.print("From ");
  IPAddress remoteIp = Udp.remoteIP();
  Serial.print(remoteIp);
  Serial.print(", port ");
  Serial.println(Udp.remotePort());

  // read the packet into packetBufffer

  int len = Udp.read(packetBuffer, 255);

  if (len > 0) {
    packetBuffer[len] = 0;
  }

  Serial.println("Contents:");
  Serial.println(packetBuffer);

  ///// SPI /////
  
  // Start SPI transaction
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(5, LOW); // Pull SS low to prep other end for transfer

    for(int p = 0;p <= packetSize;p++) {

    // Converting to byte for SPI
    byte SPIdata = packetBuffer[p];
    
    // send bit
    vspi->transfer(SPIdata);

  }

  // End SPI Transaction
  digitalWrite(5, HIGH); //pull ss high to signify end of data transfer
  vspi->endTransaction();

}

// Send recived packets to PC
void sendPackets() {

  //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
  //Udp.write(ReplyBuffer);
  //Udp.endPacket();
}
