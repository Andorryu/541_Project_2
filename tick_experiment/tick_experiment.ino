#include <arduino-timer.h>

auto timeoutTimer = timer_create_default();
Timer<>::Task timeoutHandle;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  timeoutHandle = timeoutTimer.in(5000, TimeoutHandler); // start timer
}


void loop() {
  auto ticks = timeoutTimer.tick();
  Serial.println(ticks);
}

bool TimeoutHandler(void *)
{
  
  timeoutHandle = timeoutTimer.in(5000, TimeoutHandler); // start timer
  return false; // true: repeat the action, false: to stop the task
}