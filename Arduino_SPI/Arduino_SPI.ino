#include <SPI.h>

#define MISO 50
#define test 47
volatile boolean process_it;
volatile byte pos, Slavereceived;

volatile char *inp;
volatile char *outp;
char inputBuf[100];

void setup() {

  Serial.begin(115200);

  // Pin setup
  pinMode(MISO, OUTPUT);
  //Turn on SPI in Slave Mode
  SPCR |= _BV(SPE);

  // init ptr to start of buffer;
  inp = inputBuf;
  outp = inputBuf;

  // Interuupt ON is set for SPI commnucation
  SPI.attachInterrupt();

  Serial.println("Ready for SPI");  
}

void loop() {

  if (inp != outp)
  {
    Serial.println(*outp++);
  }
}

// SPI interrupt routine
ISR (SPI_STC_vect) {
  
  *inp++ = SPDR;// Value received from master if store in variable slavereceived
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
