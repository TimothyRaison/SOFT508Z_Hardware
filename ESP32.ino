// Link to WiFi library: https://www.arduino.cc/en/Reference/WiFi

#include <SPI.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <Ethernet.h>
#include <EthernetUdp.h>

// Uninitalised pointers to SPI objects
SPIClass * vspi = NULL;

// My WIFI ssid and password
const char* ssid = "lsl001";
const char* password = "ena429okec";

//
static const int spiClk = 1000000; // 1 MHz

// Network key Index number (needed only for WEP)
int keyIndex = 0;

// local port to listen on
unsigned int localPort = 2390;

// buffer to hold incoming packet
char packetBuffer[255];

// Test buffer so I know packet has been recieved
char ReplyBuffer[] = "acknowledged";

WiFiUDP Udp;

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

  // Confirming connection to terminal, IP address, and RSSI
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.print(rssi);
  Serial.println(" dBm");

  // Starts UDP
  Udp.begin(localPort);

}

byte data = 0b01010101; // junk data to illustrate usage

void loop() {
  
  vspiCommand();
  delay(500);

}
  
//  int packetSize = Udp.parsePacket();
//  if (packetSize) {
//
//    Serial.print("Received packet of size ");
//    Serial.println(packetSize);
//    Serial.print("From ");
//    IPAddress remoteIp = Udp.remoteIP();
//    Serial.print(remoteIp);
//    Serial.print(", port ");
//    Serial.println(Udp.remotePort());
//
//    // read the packet into packetBufffer
//
//    int len = Udp.read(packetBuffer, 255);
//
//    if (len > 0) {
//      packetBuffer[len] = 0;
//    }
//
//    Serial.println("Contents:");
//    Serial.println(packetBuffer);
//
//    // Send a reply, to the IP address and port that sent us the packet we received
//    //Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
//    //Udp.write(ReplyBuffer);
//    //Udp.endPacket();
//    // covertion from char to unit_8 has a bug...?
//    
//  }
//}

// This needs changing and SS is input
void vspiCommand() {
  
  //use it as you would the regular arduino SPI API
  vspi->beginTransaction(SPISettings(spiClk, MSBFIRST, SPI_MODE0));
  digitalWrite(5, LOW); //pull SS low to prep other end for transfer
  vspi->transfer(data);  
  delay(20);
  digitalWrite(5, HIGH); //pull ss high to signify end of data transfer
  vspi->endTransaction();
}
