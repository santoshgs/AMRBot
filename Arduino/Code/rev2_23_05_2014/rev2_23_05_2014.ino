// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>

// define simple names for I/P & O/P pins
#define MOTORLEFTPOS 9
#define MOTORLEFTNEG 10
#define MOTORRIGHTPOS 5
#define MOTORRIGHTNEG 6
// pins free 4,11,12
// pins 2 & 3 are also interrupts
#define LEDPIN 13 		// built-in LED on PIN 13

// define easier names/labels for serial commands sent from the BotConnect Android app
#define RETRACE 49              // DEC 1 for toggling retrace == ASCII 49
#define STOPMOTOR 50            // DEC 2 for stopping motors == ASCII 50
#define FORWARD 102             // DEC 'f' for forward == ASCII 102
#define REVERSE 98              // DEC 'b' for back/reverse == ASCII 98
#define RIGHT 114               // DEC 'r' for right == ASCII 114
#define LEFT 108                // DEC 'l' for left == ASCII 108

byte byteRead=0; 		// buffer for serial input. Int0=pin2, Int1=Pin3
int repath[256];
int reTime[256] = {0};
int j;

// Volatile because they maybe prone to change outside loop()
volatile int elapsedTime = 0;
volatile int i=0;

// run on start/reset
void setup()
{
	Serial.begin(9600);           // set baud rate at 9600
	Serial.write("\nPower On\n");
	Serial.write("\nPlease enter:");
	Serial.write("\n\t 1. Retrace");
	Serial.write("\n\t 2. Stop motor");
	Serial.write("\n\t 3. Show Elapsed Time\n");

	pinMode(MOTORRIGHTPOS, OUTPUT);  // assign all 4 motors as output
	pinMode(MOTORRIGHTNEG, OUTPUT);
	pinMode(MOTORLEFTPOS, OUTPUT);
	pinMode(MOTORLEFTNEG, OUTPUT);
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

void runMotor(int rightPos, int rightNeg, int leftPos, int leftNeg)
{
	digitalWrite(MOTORRIGHTPOS, rightPos);
	digitalWrite(MOTORRIGHTNEG, rightNeg);
	digitalWrite(MOTORLEFTPOS, leftPos);
	digitalWrite(MOTORLEFTNEG, leftNeg);
}

void reTracePath(int a)
{
	repath[i]=a;
	reTime[i]=elapsedTime;
	elapsedTime=0;
}

void loop()
{
	while(Serial.available())
	{
		byteRead = Serial.read();    // read serial input into buffer
		switch(byteRead)
		{
		case RETRACE: // DEC 1 = ASCII 49
			Serial.write("\n\nretracting...\n\n");
			runMotor(LOW,LOW,LOW,LOW);
			delay(5000); // Delay to allow replacing robot to start position
			for(j=0;j<i;j++)
			{
				Serial.print(repath[j]);
				Serial.write("\n");
				switch(repath[j])
				{
				case 1: runMotor(HIGH,LOW,HIGH,LOW);
				break;
				case 2: runMotor(LOW,LOW,HIGH,LOW);
				break;
				case 3: runMotor(LOW,HIGH,LOW,HIGH);
				break;
				case 4: runMotor(HIGH,LOW,LOW,LOW);
				break;
				}
				delay(reTime[j+1]*1000);                                
			}
			break;                     
		case STOPMOTOR: // DEC 2 = ASCII 50
			Serial.write("\nMotors offline");
			runMotor(LOW,LOW,LOW,LOW);                        
			break;
		case 51: // DEC 5 == ASCII 53. Show time elapsed in seconds
			Serial.write("\n"); Serial.print(elapsedTime);Serial.write("s");
			break;
		case FORWARD: // DEC 'f' for forward == ASCII 102. Go forward.
			runMotor(HIGH,LOW,HIGH,LOW);
			reTracePath(1);
			i++;
			break;
		case RIGHT: // DEC 'r' for right == ASCII 114. Turn right.
			runMotor(LOW,LOW,HIGH,LOW);
			reTracePath(2);
			i++;
			break;
		case REVERSE: // DEC 'b' for back/reverse == ASCII 98. Go reverse.
			runMotor(LOW,HIGH,LOW,HIGH);
			reTracePath(3);
			i++;                        
			break;
		case LEFT: // DEC 'l' for left == ASCII 108. Turn left.
			runMotor(HIGH,LOW,LOW,LOW);
			reTracePath(4);
			i++;
			break;
		default: // wrong choice entered
			Serial.write("\nError. You entered: ");
			Serial.write(byteRead);
			break;
		}
	}
}


ISR(TIMER1_COMPA_vect)
{ 
	elapsedTime++;
}
