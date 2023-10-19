#include <arduino-timer.h>

// timeout timer
auto timeoutTimer = timer_create_default();
int timeoutms = 5000;
Timer<>::Task timeoutHandle;

// read bit timer
auto readTimer = timer_create_default();
int readTimeus = 0; // set when bitrate is calculated
Timer<1, micros>::Task readTimeHandle;

int byteCount = 0;
int** message;
int bitCount = 0;
int flagState = 0;
int bit = 0;
int lastBit = 0;
int val = 0;
int flagTimes[3]; // store time measurements here
int bitrate;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {

  auto timeLeft = timeoutTimer.tick();
  lastBit = bit; // get bit from last iter
  val = analogRead(A1);
  bit = round((float)val/900); // get new bit
  if (bit != lastBit && flagState < 4) { // if bit has flipped since last sample, and message hasn't started
    flagState++;
    if (flagState > 1) {
      flagTimes[flagState-2] = timeoutms - timeLeft;// save time between flagstate changes
      timeoutTimer.cancel(timeoutHandle); // cancel timeout if flagstate changes, but not when first bit flips
    }
    timeoutHandle = timeoutTimer.in(timeoutms, TimeoutHandler); // reset timeout timer when flagState changes
  }
  if (flagState == 4) { // finished receiving start flag - 1 0 1 0
  
    timeoutTimer.cancel(timeoutHandle); // cancel timeout when flag is done sending
    // calculate bitrate
    bitrate = avg(flagTimes, 3);
    
    Serial.print("Times: ");
    for (int i = 0; i < (sizeof(flagTimes)/sizeof(flagTimes[0])); i++) {
      Serial.print(flagTimes[i]);
      Serial.print(" ");
    }
    Serial.println();
    Serial.print("Bitrate: ");
    Serial.println(bitrate);

    readTimeHandle = readTimer.every(readTimeus, PeriodicReadHandler); // start reading
    flagState = 5;
  }

  //Serial.println("times: ");
  //for (int i = 0; i < (sizeof(flagTimes)/sizeof(flagTimes[0])); i++) {
  //  Serial.println(flagTimes[i]);
  //}
  //Serial.println();

}

bool PeriodicReadHandler(void *) {
  // increment counters
  bitCount++;
  if (bitCount == 8) {
    bitCount = 0;
    byteCount++;
  }

  // 
  
  return false;
}

bool TimeoutHandler(void *) {
  flagState = 0;
  return false;
}

int avg(int list[], int size) {
  int sum = 0;
  for (int i = 0; i < size; i++) {
    sum += list[i];
  }
  return sum/size;
}