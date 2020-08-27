// avr-libc library includes
#include <avr/io.h>
#include <avr/interrupt.h>
#include <PinChangeInt.h>
#include <PinChangeIntConfig.h>

// define simple names for I/P & O/P pins
#define MOTORLEFTPOS 9
#define MOTORLEFTNEG 10
#define MOTORRIGHTPOS 5
#define MOTORRIGHTNEG 6
// pins free 4
#define IR_right 12
#define IR_left 11
#define IR_front 2
#define IR_frontINT 0
// pins 2 & 3 are also interrupts
#define PIRPIN 7 		// assign PIR Sensor output to PIN 8
#define GASPIN 8 		// assign MQ6 Sensor output to PIN 12
#define LEDPIN 13 		// built-in LED on PIN 13

// define easier names/labels for serial commands sent from the BotConnect Android app
#define AUTOSENSE 49            // DEC 1 for toggling autosense == ASCII 49
#define STOPMOTOR 50            // DEC 2 for stopping motors == ASCII 50
#define RETRACE 51              // DEC 3 for path retrace == ASCII 51
#define FORWARD 102             // DEC 'f' for forward == ASCII 102
#define REVERSE 98              // DEC 'b' for back/reverse == ASCII 98
#define RIGHT 114               // DEC 'r' for right == ASCII 114
#define LEFT 108                // DEC 'l' for left == ASCII 108
#define PIR_SENSE 52            // DEC 4 for ASCII 52 for sensing PIR o/p
#define GAS_SENSE 53            // DEC 5 for ASCII 53 for sensing gas o/p
#define SOFTRESET 54            // DEC 6 for ASCII 54 for soft reset

byte byteRead=0; 		// buffer for serial input. Int0=pin2, Int1=Pin3
int repath[256];
int reTime[256] = {0};
int j;

// Volatile because they maybe prone to change outside loop()
volatile int elapsedTime = 0;
volatile int i=0;
volatile byte autoSense = LOW;     // used w/ interrupt. Needs to be volatile for CPU to poll this regularly
int count = 0;


// run on start/reset
void setup()
{
	Serial.begin(9600);           // set baud rate at 9600
	Serial.write("\nPower On\n");
	Serial.write("\nPlease enter:\n\t 1. Toggle Autosense\n\t 2. Stop Motors\n\t 3. Retrace\n\t 4. Sense PIR\n\t 5. Sense Gas Leak\n\t 6. Soft Reset\n");

	pinMode(MOTORRIGHTPOS, OUTPUT);  // assign all 4 motors as output
	pinMode(MOTORRIGHTNEG, OUTPUT);
	pinMode(MOTORLEFTPOS, OUTPUT);
	pinMode(MOTORLEFTNEG, OUTPUT);
	pinMode(PIRPIN, INPUT);       // assign PIN as INPUT, to read PIR's output
	pinMode(GASPIN, INPUT);       // assign PIN as INPUT, to read MQ6's output
	pinMode(LEDPIN, OUTPUT);      // LED is output

        timer1_init();
        
        pinMode(IR_front, INPUT);
        pinMode(IR_left, INPUT);
        pinMode(IR_right, INPUT);        
        // set pin2/INT0 interrupt to IR_front
        attachInterrupt(IR_frontINT, obstacleSense, RISING);
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

void obstacleSense()
{
    if(digitalRead(IR_left))
    {
     // IR_front and IR_left high
      runMotor(LOW,LOW,HIGH,LOW);     
      delay(2000);
      runMotor(LOW,LOW,LOW,LOW);
    }
    else if(digitalRead(IR_right))
    {
      // IR_front and IR_right high
      runMotor(HIGH,LOW,LOW,LOW);
      delay(4000);
      runMotor(LOW,LOW,LOW,LOW);
    }
    else
    {
      runMotor(LOW,LOW,LOW,LOW);
      delay(4000);
      runMotor(LOW,HIGH,LOW,HIGH);
      delay(2000);
      runMotor(LOW,LOW,LOW,LOW);     
    }
}

void softReset()
// Restarts program from beginning but does not reset the peripherals and registers
{
  asm volatile ("  jmp 0");  
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
		// read serial input into buffer
		byteRead = Serial.read();
		switch(byteRead)
		{
		case AUTOSENSE: // DEC 1 = ASCII 49
			Serial.write("\nAuto Sense toggled");
                        autoSense = !autoSense;
			break; 
		case RETRACE: // DEC 3 = ASCII 51
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
		case PIR_SENSE: // DEC 4 == ASCII 52
			// call PIR sensor method to check for human
			pirSensor();
			break;
		case GAS_SENSE: // DEC 5 == ASCII 53
                        // call LPG detector method to check for gas leak
			gasSensor();
			break;
		case STOPMOTOR: // DEC 2 == ASCII 50
			Serial.write("\nMotors offline");
			runMotor(LOW,LOW,LOW,LOW);                        
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
                case SOFTRESET: // DEC 6 == ASCII 54 for soft reset
                        // does not reset hardware and pins; only program
                        softReset();
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
    elapsedTime++;         // keep overall track of elapsed time
    // using to sense PIR/GAS outputs every 10 seconds
    if(autoSense == HIGH)
    {
      count++;
      if(count==10)
      {
        gasSensor();
        pirSensor();
        count = 0;       // reset count
      }
    }
}


