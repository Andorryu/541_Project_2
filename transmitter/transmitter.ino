//revised Tranmitter Code

#include <arduino-timer.h>

#define DELAY_VALUE 5000
#define LOOP 10

int bitrate = 2000;

String final_message = "";

int IR_LED_PIN = A2;
int led_blink_count = 0;
int j;

Timer<2,micros> timer;
Timer<2, micros>::Task handle;

void setup()
{
  pinMode(IR_LED_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("Setup Complete");
  digitalWrite(IR_LED_PIN, LOW);
}

void loop()
{  
  timer.tick();

  if(Serial.available() > 0)
  {
    //start_flag(LOOP); get rid of this
    String message = Serial.readString();
    int message_length = message.length();;

    Serial.println("String message: ");
    Serial.println(message);

    //prints and calculates the expected binary
    for(int i = 0; i < message_length; i++)
    {
      char character = message.charAt(i);
      byte ascii_value = int(character);
      String binary_value = byteToBinary(ascii_value);

      for(int bit = 0; bit < 8; bit++)
      {
        char current_bit = binary_value[bit]; 
        Serial.print(current_bit);
        delayMicroseconds(5000);

        if(bit == 7)
        {
          Serial.print(" ");
        }
      }
    }

    //start_flag(LOOP); instead of start loop being separete function, appedning to start value

    for(int i = 0; i < LOOP/2; i++)
    {
      final_message = final_message + "10";
    }

    for(int i = 0; i < message_length; i++)
    {
      char character = message.charAt(i);
      byte ascii_value = int(character);
      String binary_value = byteToBinary(ascii_value);
      final_message = final_message + binary_value;
    }

    //this final message is now 1010 + message in bits + end flag (00000000)
    final_message = final_message + "00000000";
    Serial.println(" ");
    Serial.println(final_message);


    handle = timer.in(DELAY_VALUE, handler);
    j = 0; 
    //blinking the actual message
        
      
      
      // if(i == final_message.length() - 1)
      // { 
      //   Serial.println("\nTotal number of times LED blinked:");
      //   Serial.println(led_blink_count);
      // }
    
  
    //to test if IR LED is not burnt, run code below and comment code out above
    //stagnant is on
    //digitalWrite(IR_LED_PIN, 1);

  }
}

/*
void start_flag(int loop)
{
  for(int i = 0; i < loop/2; i++)
  {
    digitalWrite(IR_LED_PIN, HIGH);
    delay(DELAY_VALUE);
    //delayMicroseconds(DELAY_VALUE);
    digitalWrite(IR_LED_PIN, LOW);
    delay(DELAY_VALUE);
    //delayMicroseconds(DELAY_VALUE);
  }
}
*/

bool handler(void*)
{
  handle = timer.in(DELAY_VALUE, handler);
  check_blink(final_message[j++]);
  
  if(j == final_message.length())
  {
    Serial.println("\nTotal number of times LED blinked:");
    Serial.println(led_blink_count);
    led_blink_count = 0; 
    digitalWrite(IR_LED_PIN, LOW);
    final_message = "";
    timer.cancel(handle);
  }
  return false;  
}

void check_blink(char c)
{
  if (c == '1')
  {
    // Serial.println("Led has blinked ");
    digitalWrite(IR_LED_PIN, HIGH);
    led_blink_count++;
  }
  else
  {
    // Serial.println("Led is off ");
    digitalWrite(IR_LED_PIN, LOW);
  }
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
