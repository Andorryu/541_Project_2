#define DELAY_VALUE 500
#define LOOP 10

int IR_LED_PIN = A0;
int led_blink_count = 0;

void setup()
{
  pinMode(IR_LED_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("Setup Complete");
}

void loop()
{  
  if(Serial.available() > 0)
  {
    //start_flag(LOOP);
    String message = Serial.readString();
    int message_length = message.length();;

    start_flag(LOOP);

    for(int i = 0; i < message_length; i++)
    {
      char character = message.charAt(i);
      byte ascii_value = int(character);
      String binary_value = byteToBinary(ascii_value);

      // Serial.println("Character: ");
      // Serial.println(character);
      // Serial.println("ASCII Value: \n");
      // Serial.println(ascii_value);
      // Serial.println("ASCII Value (Binary): \n");
      // Serial.println(binary_value);

      for(int bit = 0; bit < 8; bit++)
      {
        char current_bit = binary_value[bit]; 
        check_blink(current_bit);
        delay(5);
        //delayMicroseconds(DELAY_VALUE);
      }

      if(i == message_length - 1)
      { 
        Serial.println("Total number of times LED blinked: \n");
        Serial.print(led_blink_count);
      }
    }
  }
}

void start_flag(int loop)
{
  for(int i = 0; i < loop/2; i++)
  {
    digitalWrite(IR_LED_PIN, HIGH);
    delay(5);
    //delayMicroseconds(DELAY_VALUE);
    digitalWrite(IR_LED_PIN, LOW);
    delay(5);
    //delayMicroseconds(DELAY_VALUE);
  }
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
