#include <SPI.h>

#define MISO 50
#define test 47
char buf[100];
volatile boolean process_it;
volatile byte pos, Slavereceived;

void setup() {

  Serial.begin(115200);

  // Pin setup
  pinMode(MISO, OUTPUT);
  //Turn on SPI in Slave Mode
  SPCR |= _BV(SPE);

  // Interuupt ON is set for SPI commnucation
  SPI.attachInterrupt();

  Serial.println("Ready for SPI");

  process_it = false;
  
}

void loop() {

  if (process_it)
  {
    
    Serial.println(Slavereceived);
    process_it = false;
    
  }
}

// SPI interrupt routine
ISR (SPI_STC_vect) {
  
  Slavereceived = SPDR;// Value received from master if store in variable slavereceived
  process_it = true;

}

void SPIout() {

  // take the SS pin low to select the chip:
  digitalWrite(SS, LOW);
  delay(100);
  //  send in the address and value via SPI:
  SPI.transfer("potato");
  delay(100);
  // take the SS pin high to de-select the chip:
  digitalWrite(SS, HIGH);

}
