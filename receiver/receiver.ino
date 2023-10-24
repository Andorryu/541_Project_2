#include <arduino-timer.h>

#define MAX_MESSAGE_SIZE 1000
#define FLAG_LENGTH 8

// timeout timer
Timer<1, micros> timeoutTimer;
unsigned long timeoutus = 500000;
Timer<1, micros>::Task timeoutHandle;

// read bit timer
Timer<2, micros> readTimer;
unsigned long readTimeus = 0; // set when bitrate is calculated
Timer<2, micros>::Task readTimeHandle;

int byteCount = 0;
byte* message = (byte*) calloc(MAX_MESSAGE_SIZE, 1);
int bitCount = 0;
int flagState = 0;
int bit = 0;
int lastBit = 0;
int val = 0;
unsigned long flagTimes[FLAG_LENGTH-1]; // store time measurements here
float bitrate;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {

  auto timeLeft = timeoutTimer.tick();
  readTimer.tick();
  lastBit = bit; // get bit from last iter
  val = analogRead(A1);
  bit = round((float)val/900); // get new bit

  // GET START FLAG
  if (bit != lastBit && flagState < FLAG_LENGTH) { // if bit has flipped since last sample, and message hasn't started
    flagState++;
    Serial.print("FlagState: ");
    Serial.println(flagState);
    if (flagState > 1) {
      Serial.print("Time Left: ");
      Serial.println(timeLeft);
      flagTimes[flagState-2] = timeoutus - timeLeft;// save time between flagstate changes
      timeoutTimer.cancel(timeoutHandle); // cancel timeout if flagstate changes, but not when first bit flips
    }
    timeoutHandle = timeoutTimer.in(timeoutus, TimeoutHandler); // reset timeout timer when flagState changes
  }

  if (flagState == FLAG_LENGTH) { // finished receiving start flag - 1 0 1 0


    Serial.println("Flag state is 4");



    timeoutTimer.cancel(timeoutHandle); // cancel timeout when flag is done sending
    // calculate bitrate
    readTimeus = avg(flagTimes, 3);
    bitrate = 1/(usToS(readTimeus));
    
    Serial.print("Times: ");
    for (int i = 0; i < (sizeof(flagTimes)/sizeof(flagTimes[0])); i++) {
      Serial.print(flagTimes[i]);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("read time: ");
    Serial.println(readTimeus);
    Serial.print("Bitrate: ");
    Serial.println(bitrate);

    readTimeHandle = readTimer.in(readTimeus, PeriodicReadHandler); // start reading message at calculated bitrate
    flagState = FLAG_LENGTH+1;
  }
}

bool PeriodicReadHandler(void *) {

  Serial.println("READING BIT"); // 01100001 01101110 01100100 01110010 01100101 01110111 == andrew
  // read message
  message[byteCount] <<= 1;
  message[byteCount] |= bit;

  // increment counters
  bitCount++;
  if (bitCount == 8) {
    Serial.print("Byte read value: ");
    Serial.println(message[byteCount]);
    Serial.print("char read: ");
    Serial.println((char)message[byteCount]);
    bitCount = 0;
    byteCount++;
    if (message[byteCount-1] == 0) {
      int messageSize = byteCount;
      flagState = 0;
      byteCount = 0;
      Serial.println("Message:");
      printMessage(message, messageSize);
      return false;
    }
  }
  
  readTimeHandle = readTimer.in(readTimeus, PeriodicReadHandler);
  return false;
}

bool TimeoutHandler(void *) {
  flagState = 0;
  Serial.println("TIMEOUT");
  return false;
}

unsigned long avg(unsigned long list[], int size) {
  unsigned long sum = 0;
  for (int i = 0; i < size; i++) {
    sum += list[i];
  }
  return (unsigned long)(sum/size);
}

void printMessage(byte* message, int messageSize) {
  for (int i = 0; i < messageSize; i++) {
    Serial.print((char) message[i]);
  }
  Serial.println("\n");
}

float usToS(unsigned long us) {
  return ((float)us)/1000000;
}