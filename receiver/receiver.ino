#include <arduino-timer.h>

#define MAX_MESSAGE_SIZE 100
#define FLAG_LENGTH 10
#define EXPECTED_MESSAGE "hello\n"

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
  bit = val > 100 ? 1 : 0; // get new bit

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
    delayMicroseconds(50);
    // calculate bitrate
    // readTimeus = avg(flagTimes, FLAG_LENGTH-1);
    readTimeus = 2000;
    readTimeHandle = readTimer.in((unsigned long) readTimeus, PeriodicReadHandler); // start reading message at calculated bitrate
    
    // long avgTime = readTimer.tick();
    // Serial.print("Time to take average: ");
    // Serial.println(avgTime);
    // readTimeus = flagTimes[FLAG_LENGTH-4];
    message = (byte*) calloc(MAX_MESSAGE_SIZE, sizeof(char));
    //Serial.println(readTimeus);
    
    //delay(10);

    //Serial.print("Times: ");
    //for (int i = 0; i < FLAG_LENGTH-1; i++) {
      //Serial.print(flagTimes[i]);
      //Serial.print(" ");
    //}
    //Serial.println();
    //Serial.print("read time: ");
    //Serial.println(readTimeus);
    //Serial.print("Bitrate: ");
    //Serial.println(bitrate);

    flagState = FLAG_LENGTH+1;
  }
}

bool PeriodicReadHandler(void *) {

  readTimeHandle = readTimer.in(readTimeus, PeriodicReadHandler);
  // read message
    message[byteCount] <<= 1;
  // if (bitCount == 8) {
  //   message[byteCount] <<= 1;
  // }
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
      for (int i = 0; i < FLAG_LENGTH-1; i++) {
        Serial.print(flagTimes[i]);
        Serial.print(" ");
      }
      Serial.println();
      Serial.print("Bitrate: ");
      Serial.println(bitrate);
      Serial.println("Message:");
      printMessage(message, messageSize);
      // for (int i = 0; i < messageSize; i++) {
      //   Serial.print(byteToBinary(message[i]));
      //   Serial.print(" ");
      // }
      // Serial.println("");
      readTimer.cancel(readTimeHandle);
      return false;
    }
  }
  
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
  int num_error = 0;
  for (int i = 0; i < messageSize; i++) {
    String binary_string = byteToBinary(message[i]);
    String expected = byteToBinary(EXPECTED_MESSAGE[i]);
    Serial.print((char) message[i]);
    Serial.print(" - ");
    Serial.println(binary_string);
    Serial.print((char) EXPECTED_MESSAGE[i]);
    Serial.print(" - ");
    Serial.println(expected);
    num_error += comp_byte(binary_string, expected);
  }
  float ber = num_error/((float)8*(float)(messageSize-1));
  Serial.print("Expected: ");
  Serial.println(EXPECTED_MESSAGE);
  Serial.print("Received: ");
  Serial.println((char*)message);
  Serial.print("Bit Error Rate is ");
  Serial.println(ber);
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

int comp_byte(String str1, String str2) {
  int total = 0;
  for (int i = 0; i < 8; i++) {
    if (str1[i] != str2[i]) {
      total++;
    }
  }
  return total;
}