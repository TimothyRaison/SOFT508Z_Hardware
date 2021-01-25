// This code is the final scetch for the Arduino Mega, which includes:
// Communicating with the ESP32 board using SPI (full duplex)
// Interfacing with the motors on the buggy using the motor sheild mounted on the Arduino board
// Using a ultrasconic sensor to measure distance (in front of the buggy)
// Printing messages on a lcd screen

// Librarys
#include <SPI.h> // Needed for SPI communication

// Defining all the GPIO pins
#define MISO 50 // Master out slave in 
#define trigPin 19 // Trigger pin for ultrasconic sensor
#define echoPin 18 // Echo pin for ultrasconic sensor
#define aBreak 9 // Break pin for motor A
#define bBreak 8 // Break pin for motor B
#define aDirection 12 // Dirrection of motor A
#define bDirection 13 // Dirrection of motor B 
#define PWMA 3 // PWM for motor A
#define PWMB 11 // PWM for motor B
#define Encoder 3 // Encoder Pin (ISR)

// Setting up input buffer
// Defining the buffer size
int bufferSize = 20;

// Two pointers for a circular buffer(*inp pointer will chase *outp)
volatile char *inp;
volatile char *outp;

// Input from SPI storage buffer
char inputBuf[20];

// variable that saves current action
char action;

// Global variables for the
int distance;

// Records the number of encoder turns
volatile int wheelEncoder = 0;

// Variables to work out rpm
unsigned long lastPulseTime = 0;
unsigned long rpm = 0;

void setup() {

  pinMode(trigPin, OUTPUT); // Ultrasconic trig
  pinMode(echoPin, INPUT); // Ultrasconic echo
  // Set high to stop (shorts motor terminals together)
  pinMode(aBreak, OUTPUT); // Channel A Brake
  pinMode(bBreak, OUTPUT); // Channel B Brake
  // HIGH is backwards, LOW is forward
  pinMode(aDirection, OUTPUT); // Channel A Direction
  pinMode(bDirection, OUTPUT); // Channel B Direction
  // Optical encoder sensor inputs
  // Channel A leads B for forward, B leading A for back
  pinMode(Encoder, INPUT); // Wheel encoder

  // Calling buggy stop at start of code to make sure that the buggy never starts moving right away just in case of a bug
  BuggyStop();

  // Start serial UART
  Serial.begin(115200);

  // Setting the MISO as a output (Master in slave out)
  pinMode(MISO, OUTPUT);

  // Turn on SPI in Slave Mode (setting register)
  SPCR |= _BV(SPE);

  // Interuupt ON is set for SPI communication
  SPI.attachInterrupt();

  // ISR for recording the encoder pulses
  attachInterrupt(digitalPinToInterrupt(Encoder), RPM, RISING);

  // Print to serial saying when SPI is ready (used for testing)
  //Serial.println("Ready for SPI");

}

// SPI interrupt routine (called when master sends a bit)
ISR (SPI_STC_vect) {

  // Variable to record the recived value
  uint8_t value = SPDR;

  // Switch to decode the message
  switch (value) {
    // If a 0 ignore (0 is sent as a null bit when only wanting to recive)
    case 0:
      // Ignore "null" commands
      break;
    // If reciving a 1, ESP wants the current distance
    case '1':
      // Setting something to the SPDR sends it automatically
      SPDR = (uint8_t)distance;
      break;
    // If reciving a 2, ESP wants the current RPM
    case '2':
      // Setting something to the SPDR sends it automatically
      SPDR = (uint8_t)rpm;
      break;
    // If reciving a 3, ESP wants the current action
    case '3':
      // Setting something to the SPDR sends it automatically
      SPDR = action;
      break;
    // If none of the above commands, put the message in action buffer
    default:
      // Value received from master if store in variable inp (pointer for circular buffer)
      *inp++ = value;

      // If inp gets to end of buffer loop back to start (so no bufer overflow)
      if (inp == inputBuf + bufferSize) {
        // Point to start of array
        inp = inputBuf;
      }
  }
}

// Source: https://create.arduino.cc/projecthub/Isaac100/getting-started-with-the-hc-sr04-ultrasonic-sensor-036380
// Fuction that returns the buggys distance reading from the ultrasconic sensor
// Updates global variable distance
void Ultrasconic() {

  // Looking at the HC SR04 datasheet
  // Formula: uS / 58 = centimeters
  // "We suggest to use over 60ms measurement cycle, in order to prevent trigger signal to the echo signal"

  // long is 64 bit size number
  long duration;

  // Send trigger pin pulse in the pattern that was specified on the datatsheet
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Waiting for the response and noting the diration
  duration = pulseIn(echoPin, HIGH);

  // Using the formula from the datasheet to work out distance
  distance = (duration / 2) / 29.1;

  // Print the distance to the terminal for testing (if too close or far print out of range)
  // This if was for testing
  if (distance >= 200 || distance <= 0) {

    //Serial.println("Out of range");
  }
  else {

    // Print distance for testing
    //Serial.print(distance);

    //Serial.println(" cm");
    if (distance < 10) {

      // Print for testing
      //Serial.println("STOP BUGGY");
      // Stop the buggy if distance is too low
      BuggyStop();

    }
  }
}

// Source: https://forum.arduino.cc/index.php?topic=71739.msg540732#msg540732
// Measures how many pulses from the encoders have happened (pulses recorded by ISR)
// Updates global variable RPM
void RPM() {

  // Gets time since program started
  unsigned long now = micros();
  // Working out the interval (time since started minus time last time a pulse was recorded)
  unsigned long interval = now - lastPulseTime;

  // If the interfal is less than 2ms then most likely a fulty reading
  if (interval > 2000)
  {
    // Formula to work out rpm
    // Dividing by 19, as there is 19 slots in the encoder wheel
    rpm = 60000000UL / interval / 19;
    // Set when pulse happened so I can refer to it in the future
    lastPulseTime = now;
  }
}

// Main loop (Forever loop, if I had not run out of time I would have threaded/event manager)
void loop() {

  //test that will print the current rpm to the serial terminal (rpm is updated by ISR triggered by encoder)
  //Serial.println(rpm);

  // Updatting the distnace variable to actual distance
  Ultrasconic();

  // Making sure that the rpm is 0, if the buggy is not moving.
  // Stopping false encoder readings

  // Gets time since program started
  unsigned long now = micros();

  // Working out the interval (time since started minus time last time a pulse was recorded)
  unsigned long interval = now - lastPulseTime;

  // If interval gets larger than 1s then set rpm to 0
  if (interval > 1000000) {
    rpm = 0;
  }

  // Checking if any actions have been recived
  // Ouput pointer chasing input pointer in circular buffer (means that there is data to be sent)
  if (inp != outp)
  {

    // Grab next char...
    char x = *outp;

    // If outp gets to of buffer loop back to start
    outp++;
    if (outp == inputBuf + bufferSize) {
      // point to start of array
      outp = inputBuf;

      // Prints used for testing the circular buffer
      //Serial.println("outp reset");

    }

    // if x is any of the input commands
    // All the serial prints are from testing
    switch (x) {

      // Stop command
      case 's':
        BuggyStop();
        //Serial.println("STOP");
        break;

      // Forward command
      case 'f':
        BuggyForward();
        //Serial.println("Forward");
        break;

      // Left command
      case 'l':
        BuggyLeft();
        //Serial.println("LEFT");
        break;

      // Right command
      case 'r':
        BuggyRight();
        //Serial.println("RIGHT");
        break;

      // Backwards command
      case 'b':
        BuggyBack();
        //Serial.println("BACK");
        break;

      // Not valid command
      default:
        //Serial.println(x);
        //Serial.println("unknown command");
    }
  }
}

// Stopping the buggy
void BuggyStop() {

  // Setting the break high to stop the buggy
  digitalWrite(aBreak, HIGH);
  digitalWrite(bBreak, HIGH);
  // When stopped reset rpm to 0
  rpm = 0;
  lastPulseTime = 0;

  // Set current action to not moving to be returned to ESP32
  action = 's';

}

// Forward command
void BuggyForward() {

  // Make sure stop is high before programming pins
  digitalWrite(aBreak, HIGH);
  digitalWrite(bBreak, HIGH);

  // Setting speed
  // Value from 0 and 256 to control PWM frequency leading to motor speed
  digitalWrite(PWMA, 255);
  digitalWrite(PWMB, 255);

  // Setting direction of the buggy
  // HIGH is backwards, LOW is forward
  digitalWrite(aDirection, LOW);
  digitalWrite(bDirection, LOW);

  // Stopping break
  digitalWrite(aBreak, LOW);
  digitalWrite(bBreak, LOW);

  // Set current action to forward to be returned to ESP32
  action = 'f';

}

// Turning right
void BuggyRight() {

  // Make sure stop is high before programming pins
  digitalWrite(aBreak, HIGH);
  digitalWrite(bBreak, HIGH);

  // Setting speed
  digitalWrite(PWMA, 255);
  digitalWrite(PWMB, 255);

  // Setting direction of the buggy
  digitalWrite(aDirection, HIGH);
  digitalWrite(bDirection, LOW);

  // Stopping break
  digitalWrite(aBreak, LOW);
  digitalWrite(bBreak, LOW);

  // Set current action to right turn to be returned to ESP32
  action = 'r';

}

// Turning left
void BuggyLeft() {

  // Make sure stop is high before programming pins
  digitalWrite(aBreak, HIGH);
  digitalWrite(bBreak, HIGH);

  // Setting speed
  digitalWrite(PWMA, 255);
  digitalWrite(PWMB, 255);

  // Setting directiong of the buggy
  digitalWrite(aDirection, LOW);
  digitalWrite(bDirection, HIGH);

  // Stopping break
  digitalWrite(aBreak, LOW);
  digitalWrite(bBreak, LOW);

  // Set current action to left turn to be returned to ESP32
  action = 'l';

}

// reverse
void BuggyBack() {

  // Make sure stop is high before programming pins
  digitalWrite(aBreak, HIGH);
  digitalWrite(bBreak, HIGH);

  // Setting speed
  digitalWrite(PWMA, 255);
  digitalWrite(PWMB, 255);

  // Setting directiong of the buggy
  digitalWrite(aDirection, HIGH);
  digitalWrite(bDirection, HIGH);

  // Stopping break
  digitalWrite(aBreak, LOW);
  digitalWrite(bBreak, LOW);

  // Set current action to backwards to be returned to ESP32
  action = 'b';

}
