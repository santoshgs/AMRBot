// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

// define simple names for I/P & O/P pins
#define MOTORLEFT 5
#define MOTORLEFT2 10
#define MOTORRIGHT 6
#define MOTORRIGHT2 11

#define LEDPIN 13 		// built-in LED on PIN 13
#define PIRPIN 8 		// assign PIR Sensor output to PIN 8
#define GASPIN 12 		// assign MQ6 Sensor output to PIN 12

// define easier names for serial commands sent from the BotConnect Android app
#define FORWARD 102             // DEC 'f' for forward == ASCII 102
#define REVERSE 98              // DEC 'b' for back/reverse == ASCII 98
#define RIGHT 114               // DEC 'r' for right == ASCII 114
#define LEFT 108                // DEC 'l' for left == ASCII 108
#define PIR_SENSE 51            // ASCII 51 for sensing PIR o/p
#define GAS_SENSE 52            // ASCII 52 for sensing gas o/p

byte byteRead; 			// buffer for serial input
int seconds = 0;

// run on start/reset
void setup()
{
	Serial.begin(9600); // set baud rate at 9600
	Serial.write("\nPower On\n");
	Serial.write("\nPlease enter:\n\t 1. Start motor\n\t 2. Stop motor\n\t 3. Sense PIR\n\t 4. Sense Gas Leak\n");

	pinMode(MOTORRIGHT, OUTPUT); // assign all 4 motors as output
	pinMode(MOTORRIGHT2, OUTPUT);
	pinMode(MOTORLEFT, OUTPUT);
	pinMode(MOTORLEFT2, OUTPUT);
	pinMode(PIRPIN, INPUT); // assign PIN 8 as INPUT, to read PIR's output
	pinMode(GASPIN, INPUT); // assign PIN 12 as INPUT, to read MQ6's output
	pinMode(LEDPIN, OUTPUT); // LED is output

        timer1_init();
}

void timer1_init()
{
        // initialize Timer1
	cli();          // disable global interrupts
	TCCR1A = 0;     // set entire TCCR1A register to 0
	TCCR1B = 0;     // same for TCCR1B
 
        // set compare match register to desired timer count:
	OCR1A = 15624;  // 15264 is value for 1 second. See: http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/
        // turn on CTC mode:
	TCCR1B |= (1 << WGM12);
	// Set CS10 and CS12 bits for 1024 prescaler:
	TCCR1B |= (1 << CS10);
	TCCR1B |= (1 << CS12);
        // enable timer compare interrupt:
	TIMSK1 |= (1 << OCIE1A);
        // enable global interrupts:
	sei();
}

void forwardMotor()
{
	Serial.write("\nMoving forward...");
  	digitalWrite(MOTORLEFT, HIGH);
	digitalWrite(MOTORLEFT2, LOW);
	digitalWrite(MOTORRIGHT, HIGH);
	digitalWrite(MOTORRIGHT2, LOW);
}

void turnLeft()
{
	Serial.write("\nTurning left...");
  	digitalWrite(MOTORLEFT, LOW);
	digitalWrite(MOTORLEFT2, LOW);
	digitalWrite(MOTORRIGHT, HIGH);
	digitalWrite(MOTORRIGHT2, LOW);
}

void turnRight()
{
	Serial.write("\nTurning right...");  
	digitalWrite(MOTORLEFT, HIGH);
	digitalWrite(MOTORLEFT2, LOW);
	digitalWrite(MOTORRIGHT, LOW);
	digitalWrite(MOTORRIGHT2, LOW);
}

void reverseMotor()
{
  	Serial.write("\nMoving back...");
	digitalWrite(MOTORLEFT, LOW);
	digitalWrite(MOTORLEFT2, HIGH);
	digitalWrite(MOTORRIGHT, LOW);
	digitalWrite(MOTORRIGHT2, HIGH);
}

void pirSensor()
{
	// glow LED if output of PIR is high
	if(digitalRead(PIRPIN))
	{
		digitalWrite(LEDPIN, HIGH);
		Serial.write("\nHuman detected! Say hi. ;)");
	}
	else 
	{
		digitalWrite(LEDPIN, LOW);
		Serial.write("\nHuman absent. :(");
	}
}

void gasSensor()
{
        // glow LED if output of GASPIN is high
	if(digitalRead(GASPIN))
	{
		digitalWrite(LEDPIN, HIGH);
		Serial.write("\nOMG! Gas leak. Run for your lives. :P");
	}
	else
	{
		digitalWrite(LEDPIN, LOW);
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
			digitalWrite(MOTORLEFT, HIGH);
			digitalWrite(MOTORLEFT2, LOW);
			delay(5000);
			digitalWrite(MOTORLEFT, LOW);
			digitalWrite(MOTORLEFT2, HIGH);
			break; 
		case 50: // DEC 2 = ASCII 50
			Serial.write("\nMotor 1 offline");
			digitalWrite(MOTORLEFT, LOW);
			digitalWrite(MOTORLEFT2, LOW);
                        break;
		case PIR_SENSE: // DEC 3 == ASCII 51
			// call PIR sensor method to check for human
			pirSensor();
			break;
		case GAS_SENSE: // DEC 4 == ASCII 52
                        // call LPG detector method to check for gas leak
			gasSensor();
			break;
		case FORWARD: // DEC 'f' for forward == ASCII 102
			forwardMotor();
			break;
		case LEFT: // DEC 'l' for left == ASCII 108
			turnLeft();
			break;
		case RIGHT: // DEC 'r' for right == ASCII 114
			turnRight();
			break;
		case REVERSE: // DEC 'b' for back/reverse == ASCII 98
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

 
ISR(TIMER1_COMPA_vect)
{ // using to sense PIR/GAS outputs every 10 seconds
    seconds++;
    if(seconds == 10)
    {
      seconds = 0;
      gasSensor();
      pirSensor();
    }
}
