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
	Serial.write("\nPlease enter:\n\t 1. Start motor\n\t 2. Stop motor\n\t 3. Sense PIR\n\t 4. Sense Gas Leak\n");
	pinMode(motorRight, OUTPUT); // assign all 4 motors as output
	pinMode(motorRight2, OUTPUT);
	pinMode(motorLeft, OUTPUT);
	pinMode(motorLeft2, OUTPUT);
	pinMode(pir, INPUT); // assign PIN 8 as INPUT, to read PIR's output
	pinMode(gas, INPUT); // assign PIN 12 as INPUT, to read MQ6's output
	pinMode(led, OUTPUT); // LED is output
}

void forwardMotor()
{
	Serial.write("\nMoving forward...");
  	digitalWrite(motorLeft, HIGH);
	digitalWrite(motorLeft2, LOW);
	digitalWrite(motorRight, HIGH);
	digitalWrite(motorRight2, LOW);
}

void turnLeft()
{
	Serial.write("\nTurning left...");
  	digitalWrite(motorLeft, LOW);
	digitalWrite(motorLeft2, LOW);
	digitalWrite(motorRight, HIGH);
	digitalWrite(motorRight2, LOW);
}

void turnRight()
{
	Serial.write("\nTurning right...");  
	digitalWrite(motorLeft, HIGH);
	digitalWrite(motorLeft2, LOW);
	digitalWrite(motorRight, LOW);
	digitalWrite(motorRight2, LOW);
}

void reverseMotor()
{
  	Serial.write("\nMoving back...");
	digitalWrite(motorLeft, LOW);
	digitalWrite(motorLeft2, HIGH);
	digitalWrite(motorRight, LOW);
	digitalWrite(motorRight2, HIGH);
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
		digitalWrite(led, LOW);
		Serial.write("\nHuman absent. :(");
	}
}

void gasSensor()
{
	if(digitalRead(gas) == HIGH)
	{
		digitalWrite(led, HIGH);
		Serial.write("\nOMG! Gas leak. Run for your lives. :P");
	}
	else
	{
		digitalWrite(led, LOW);
		Serial.write("\nZzzz... no gas leak detected.");
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
			Serial.write("\nMotor 1 online");
			digitalWrite(motorLeft, HIGH);
			digitalWrite(motorLeft2, LOW);
			delay(5000);
			digitalWrite(motorLeft, LOW);
			digitalWrite(motorLeft2, HIGH);
			break; 
		case 50: // DEC 2 = ASCII 50
			Serial.write("\nMotor 1 offline");
			digitalWrite(motorLeft, LOW);
			digitalWrite(motorLeft2, LOW);
                        break;
		case 51: // DEC 3 == ASCII 51
			// call PIR sensor method to check for human
			pirSensor();
			break;
		case 52: // DEC 4 == ASCII 52
                        // call LPG detector method to check for gas leak
			gasSensor();
			break;
		case 102: // DEC 'f' for forward == ASCII 102
			forwardMotor();
			break;
		case 108: // DEC 'l' for left == ASCII 108
			turnLeft();
			break;
		case 114: // DEC 'r' for right == ASCII 114
			turnRight();
			break;
		case 98: // DEC 'b' for back/reverse == ASCII 98
			reverseMotor();
			break;
		default: // other input value entered
			Serial.write("\nError. You entered: ");
			Serial.write(byteRead);
			// Serial.print(byteRead); // prints AS
			break;
		}
	}
}
