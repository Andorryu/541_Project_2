#include <arduino-timer.h>

#define MAX_MESSAGE_SIZE 100
#define FLAG_LENGTH 4

// timeout timer
Timer<1, micros> timeoutTimer;
unsigned long timeoutus = 1000000;
Timer<1, micros>::Task timeoutHandle;

// read bit timer
Timer<2, micros> readTimer;
unsigned short readTimeus = 0; // set when bitrate is calculated
unsigned short flagTimes[FLAG_LENGTH-1]; // store time measurements here
Timer<2, micros>::Task readTimeHandle;

bool throw_bit = true;
int byteCount = 0;
byte* message = NULL;
int bitCount = 0;
int flagState = 0;
int bit = 0;
int lastBit = 0;
int val = 0;
float bitrate;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Setup Complete");
}

void loop() {

  auto timeLeft = timeoutTimer.tick();
  readTimer.tick();
  lastBit = bit; // get bit from last iter
  val = analogRead(A1);
  bit = round((float)val/550); // get new bit

  // GET START FLAG
  if (bit != lastBit && flagState < FLAG_LENGTH) { // if bit has flipped since last sample, and message hasn't started
    flagState++;
    // Serial.print("FlagState: ");
    // Serial.println(flagState);
    if (flagState > 1) {
      //Serial.print("Time Left: ");
      //Serial.println(timeLeft);
      flagTimes[flagState-2] = timeoutus - timeLeft;// save time between flagstate changes
      timeoutTimer.cancel(timeoutHandle); // cancel timeout if flagstate changes, but not when first bit flips
    }
    timeoutHandle = timeoutTimer.in(timeoutus, TimeoutHandler); // reset timeout timer when flagState changes
  }

  if (flagState == FLAG_LENGTH) { // finished receiving start flag - 1 0 1 0


    //Serial.print("Flag state is ");
    //Serial.println(FLAG_LENGTH);

    timeoutTimer.cancel(timeoutHandle); // cancel timeout when flag is done sending
    
    // calculate bitrate
    //readTimeus = avg(flagTimes, FLAG_LENGTH-1);
    // readTimeus = flagTimes[FLAG_LENGTH-4];
    readTimeus = 500;
    // Serial.println(readTimeus);
    readTimeHandle = readTimer.in((unsigned long) readTimeus, PeriodicReadHandler); // start reading message at calculated bitrate
    
    //delay(10);

    //Serial.print("Times: ");
    for (int i = 0; i < FLAG_LENGTH-1; i++) {
      //Serial.print(flagTimes[i]);
      //Serial.print(" ");
    }
    //Serial.println();
    //Serial.print("read time: ");
    //Serial.println(readTimeus);
    //Serial.print("Bitrate: ");
    //Serial.println(bitrate);

    flagState = FLAG_LENGTH+1;
  }
}

bool PeriodicReadHandler(void *) {

  // read message
  if (throw_bit) {
    delayMicroseconds(200);
    throw_bit = false;
    message = (byte*) calloc(MAX_MESSAGE_SIZE, sizeof(char));
  }
  message[byteCount] <<= 1;
  message[byteCount] |= bit;

  // increment counters
  bitCount++;
  if (bitCount == 8) {
    bitCount = 0;
    byteCount++;
    if (message[byteCount-1] == 0) {
      int messageSize = byteCount;
      flagState = 0;
      byteCount = 0;
      bitrate = 1/(usToS(readTimeus));
      // for (int i = 0; i < FLAG_LENGTH-1) {

      // }
      Serial.print("Bitrate: ");
      Serial.println(bitrate);
      Serial.println("Message:");
      printMessage(message, messageSize);
      // for (int i = 0; i < messageSize; i++) {
      //   Serial.print(byteToBinary(message[i]));
      //   Serial.print(" ");
      // }
      //Serial.println("");
      
      throw_bit = true;
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

unsigned short avg(unsigned short list[], int size) {
  unsigned short sum = 0;
  for (int i = 0; i < size; i++) {
    sum += list[i];
  }
  return (unsigned short)(sum/size);
}

void printMessage(byte* message, int messageSize) {
  for (int i = 0; i < messageSize; i++) {
    Serial.print((char) message[i]);
    Serial.print(" - ");
    Serial.println(byteToBinary(message[i]));
  }
  free(message);
}

float usToS(unsigned long us) {
  return ((float)us)/1000000;
}

String byteToBinary(byte num) 
{
  String binaryString = "";
  for (int i = 7; i >= 0; i--) 
  {
    int bit = (num >> i) & 1;
    binaryString += String(bit);
  }
  return binaryString;
}