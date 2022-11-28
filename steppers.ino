#include <MIDI.h>
#include "pitches.h"

//ARDUINO PINS
//configured for CNC Shield V3
#define stepPin_M1 2
#define stepPin_M2 3
#define stepPin_M3 4
#define stepPin_M4 5
#define stepPin_M5 6


//note that Direction Pins aren't necessary
//comment out if you don't plan on using them
#define dirPin_M1 5 
#define dirPin_M2 6
#define dirPin_M3 7
#define dirPin_M4 13

#define enPin 8 //Steppers are enabled when EN pin is pulled LOW

#define TIMEOUT 10000 //Number of milliseconds for watchdog timer

//The index corresponds to the MIDI channel/Motor number being used. Index 0 is not used.
unsigned long motorSpeeds[] = {0, 0, 0, 0, 0, 0}; //holds the speeds of the motors. 
unsigned long prevStepMicros[] = {0, 0, 0, 0, 0, 0}; //last time
const bool motorDirection = LOW; //you can use this to change the motor direction, comment out if you aren't using it.
bool disableSteppers = HIGH; //status of the enable pin. disabled when HIGH. Gets enabled when the first note on message is received.
unsigned long WDT; //Will store the time that the last event occured.

MIDI_CREATE_DEFAULT_INSTANCE(); //use default MIDI settings

void setup() 
{
  pinMode(stepPin_M1, OUTPUT);
  pinMode(stepPin_M2, OUTPUT);
  pinMode(stepPin_M3, OUTPUT);
  pinMode(stepPin_M4, OUTPUT);
  pinMode(stepPin_M5, OUTPUT);
  

  pinMode(enPin, OUTPUT);

  MIDI.begin(MIDI_CHANNEL_OMNI); //listen to all MIDI channels
  MIDI.setHandleNoteOn(handleNoteOn); //execute function when note on message is recieved
  MIDI.setHandleNoteOff(handleNoteOff); //execute function when note off message is recieved
  //Serial.begin(115200); //allows for serial MIDI communication, comment out if using HIDUINO or LUFA
}

void loop() 
  {
      MIDI.read(); //read MIDI messages
      digitalWrite(enPin, disableSteppers); //choose whether to enable or disable steppers.
      singleStep(1, stepPin_M1); //run each stepper at specified speed
      singleStep(2, stepPin_M2);
      singleStep(3, stepPin_M3);
      singleStep(4, stepPin_M4);
      singleStep(5, stepPin_M5);

      if (millis() - WDT >= TIMEOUT)
      {
        disableSteppers = HIGH; //When the time has elapsed, disable the steppers
      }
  }


void handleNoteOn(byte channel, byte pitch, byte velocity) //MIDI Note ON Command
  {
      disableSteppers = LOW; //enable steppers. 
      motorSpeeds[channel] = pitchVals[pitch]; //set the motor speed to specified pitch
      /*
       * Potentially add a grid of steppers and use the MIDI velocity 
       * control the number of active steppers in each collumn
       * Each collumn requires its own MIDI channel.
       */
  }


void handleNoteOff(byte channel, byte pitch, byte velocity) //MIDI Note OFF Command
  {
     motorSpeeds[channel] = 0; //set motor speed to zero
  }

void singleStep(byte motorNum, byte stepPin)
  {
      if ((micros() - prevStepMicros[motorNum] >= motorSpeeds[motorNum]) && (motorSpeeds[motorNum] != 0)) 
      { //step when correct time has passed and the motor is at a nonzero speed
          prevStepMicros[motorNum] += motorSpeeds[motorNum];
          WDT = millis(); //update watchdog timer
          digitalWrite(stepPin, HIGH);
          digitalWrite(stepPin, LOW);
      }
  }
