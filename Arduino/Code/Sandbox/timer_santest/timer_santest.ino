
#define LEDPIN 13
#define infra1 12
#define infra2 11
#define infra3 10
byte byteRead;

void setup()
{
    Serial.begin(9600); // set baud rate at 9600
    Serial.write("\nPower On\n");
    pinMode(LEDPIN, OUTPUT);
    pinMode(infra1, INPUT);
    pinMode(infra2, INPUT);
    pinMode(infra3, INPUT);    
}

void loop()
{
 while(Serial.available())
 {
   delay(5000);
   byteRead = Serial.read();
   if((digitalRead(infra1)) && (digitalRead(infra2)) == HIGH)
     {
       Serial.write("IR1 and IR2 high\n");
     }
   else
  {
    Serial.write("IR1 and IR2 low\n");  
  }
  if((digitalRead(infra2)) && (digitalRead(infra3)) == HIGH)
  {
    Serial.write("IR2 and IR3 high\n");
  }
  else 
  {
    Serial.write("IR2 and IR3 low\n");
  }
 }
}
