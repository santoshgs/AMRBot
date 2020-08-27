// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

// define simple names for I/P & O/P pins
#define MOTORLEFT 9
#define MOTORLEFT2 10
#define MOTORRIGHT 5
#define MOTORRIGHT2 6
// pins free 4,11,12
// pins 2 & 3 are also interrupts
#define PIRPIN 7 		// assign PIR Sensor output to PIN 8
#define GASPIN 8 		// assign MQ6 Sensor output to PIN 12
#define LEDPIN 13 		// built-in LED on PIN 13

// define easier names/labels for serial commands sent from the BotConnect Android app
#define FORWARD 102             // DEC 'f' for forward == ASCII 102
#define REVERSE 98              // DEC 'b' for back/reverse == ASCII 98
#define RIGHT 114               // DEC 'r' for right == ASCII 114
#define LEFT 108                // DEC 'l' for left == ASCII 108
#define PIR_SENSE 51            // ASCII 51 for sensing PIR o/p
#define GAS_SENSE 52            // ASCII 52 for sensing gas o/p

byte byteRead=0; 			// buffer for serial input. Int0=pin2, Int1=Pin3
int seconds = 0;
int count = 0;
volatile int autoSense = 0;     // used w/ interrupt. Needs to be volatile for CPU to poll this regularly

// run on start/reset
void setup()
{
	Serial.begin(9600);           // set baud rate at 9600
	Serial.write("\nPower On\n");
	Serial.write("\nPlease enter:\n\t 1. Toggle Autosense\n\t 2. Stop motor\n\t 3. Sense PIR\n\t 4. Sense Gas Leak\n");

	pinMode(MOTORRIGHT, OUTPUT);  // assign all 4 motors as output
	pinMode(MOTORRIGHT2, OUTPUT);
	pinMode(MOTORLEFT, OUTPUT);
	pinMode(MOTORLEFT2, OUTPUT);
	pinMode(PIRPIN, INPUT);       // assign PIN as INPUT, to read PIR's output
	pinMode(GASPIN, INPUT);       // assign PIN as INPUT, to read MQ6's output
	pinMode(LEDPIN, OUTPUT);      // LED is output

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
			Serial.write("\nAuto Sense toggled");
                        autoSense = !autoSense;
			break; 
		case 50: // DEC 2 = ASCII 50
			Serial.write("\nMotors offline");
			digitalWrite(MOTORRIGHT, LOW);
			digitalWrite(MOTORRIGHT2, LOW);
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
{ 
    // seconds++;         // keep overall track of elapsed time
    count++;
    // using to sense PIR/GAS outputs every 10 seconds
    if((count == 10) && (autoSense == 1))
    {
      count = 0;       // reset count
      gasSensor();
      pirSensor();
    }
}
