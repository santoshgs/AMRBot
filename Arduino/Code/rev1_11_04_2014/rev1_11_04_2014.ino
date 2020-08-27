int motorLeft = 5;
int motorLeft2 = 10;
int motorRight = 6;
int motorRight2 = 11;
byte byteRead; // buffer for serial input
int pir = 8; // assign PIR Sensor output to PIN 8
int gas = 12; // assign MQ6 Sensor output to PIN 12
int led = 13; // built-in LED on PIN 13

// run on start/reset
void setup()
{
  Serial.begin(9600); // set baud rate at 9600
  Serial.write("\nPower On\n");
  Serial.write("\nPlease enter:\n 1. Start motor.\n 2. Stop motor.\n 3. Sense PIR.\n 4. Sense Gas Leak. \n");
  pinMode(motorRight, OUTPUT); // assign all 4 motors as output
  pinMode(motorRight2, OUTPUT);
  pinMode(motorLeft, OUTPUT);
  pinMode(motorLeft2, OUTPUT);
  pinMode(pir, INPUT); // assign PIN 8 as INPUT, to read PIR's output
  pinMode(gas, INPUT); // assign PIN 12 as INPUT, to read MQ6's output
  pinMode(led, OUTPUT); // LED is output
}

void pirSensor()
{
    // glow LED if output of PIR is high
    if(digitalRead(pir) == HIGH)
     {
       digitalWrite(led, HIGH);
       Serial.write("\nHuman detected! Say hi. ;)");
     }
    else 
     {
       digitalWrite(led,LOW);
       Serial.write("\nHuman absent. :(");
     }
}

void gasSensor()
{
    if(digitalRead(gas) == HIGH)
    {
       digitalWrite(led, HIGH);
    }
    else
    {
       digitalWrite(led, LOW);
    }
}

void loop()
{
  while(Serial.available())
  {
    // read serial input into buffer
    byteRead = Serial.read();
    switch(byteRead)
    {
      case 49: // DEC 1 = ASCII 49
               Serial.write("\nMotor 1 online\n");
               digitalWrite(motorLeft, HIGH);
               digitalWrite(motorLeft2, LOW);
               delay(5000);
               digitalWrite(motorLeft, LOW);
               digitalWrite(motorLeft2, HIGH);
               break; 
      case 50: // DEC 2 = ASCII 50
               Serial.write("\nMotor 1 is offline");
               digitalWrite(motorLeft, LOW);
               digitalWrite(motorLeft2, LOW);
      case 51: // DEC 3 == ASCII 51
               // call PIR sensor method to check for human
               pirSensor();
               break;
      case 52: // DEC 4 == ASCII 52
               gasSensor();
               break;
      default: // wrong input entered
               Serial.write("\nError. You entered: ");
               Serial.write(byteRead);
               Serial.print(byteRead);
               break;
    }
  }
}
