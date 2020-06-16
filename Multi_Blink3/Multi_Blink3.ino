/*
 Multi_Blink3

 Blink lots of LEDs at different frequencies simultaneously, include fades, delays 
 and loops in patterns

 Marco Colli - September 2012
 
 Demonstrates 
  - the way to carry out multiple time based tasks without using the delay() function
  - the use of structures (and structures within structures)
  - a data driven approach to programming to create compact, reusable code

 How this works
 ==============
   This sketch separates the HOW (code) and the WHAT (data) of multi-tasking 
   the blinking of LEDs. How we blink LEDS is the same no matter what LED we are 
   blinking, so this can be defined independently. What outputs the blinking applies 
   to and how long it lasts is separately defined and applied to the code.
   This data driven approach approach is applicable to any number of situations 
   where repeated independent actions need to be carried out simultaneously.
   
   The code used in this sketch implements a general approach to manipulating 
   digital outputs based on a Finite State Machine. What to do in each state of the 
   FSM is defined using the constants MB_*, arranged in data tables that are 'run'
   by the FSM. To change the behaviour of the software, the data table is modified 
   for the new requirements.

   The data table (defined in T[] below) consists of an arbitrary length array of 
   ledTable structures that define the parameters for the digital output and 
   'remember' the current FSM parameters - its current state and the time for the
   next state transition.

   Each ledTable entry has associated with it up to MAX_STATE states (defined 
   using stateDef structures) that specify what the FSM will do when it is in 
   that state. To change the number of allowable states, modify the MAX_STATE 
   constant. The state value is just the array subscript.

   The tables can be statically or dynamically initialised. In this code the 
   patterns are defined statically in the T[] tables.

 Defining stateDef entries
 =========================
  - MB_HIGH and MB_LOW
    ------------------
    Keep the LED on or off for the specified time. The entry 
	  {MB_HIGH, 25, 0} 
	keeps the digital output high for 25 milliseconds.

  - MB_LOOP
    -------
    Makes the pattern loop back to the nominated array element in the pattern for the 
	specified number of times. 

	The macro LOOP_PARAM(s, c) is used to pack the loop parameters into the data word, 
	where s is the state to loop back to and n is the total number of times to execute 
	the loop. Macros LOOP_STATE_GET() and LOOP_SP_GET() are used to unpack this data.

	The entry
	  {MB_LOOP, LOOP_PARAM(0, 3}
	causes the FSM to loop back to element 0 and repeat it 2 more time (3 total).

  - MB_FADE_ON and MB_FADE_OFF
    --------------------------
	Fade a PWM output from a starting value to an end value (both a number 0 to 255), 
	keeping each state for the specified time.  This is, in effect, a special kind 
	of loop.

	The macro FADE_PARAM(s, e) is used to pack the fade effect parameters into the data 
	word, where s is the starting PWM value and e is the end PWM value. The macros 
	FADE_START_GET() and FADE_END_GET() are used to unpack these data.

	The entry 
	  {MB_FADE_OFF, 45, FADE_PARAM(30, 1), 0} 
	fades a PWM output from a starting value of 30 to 1, cycling one PWM value each 
	45 milliseconds.

 Building a ledTable
 ===================
    Whilst ledTAble entries are defined and run independently, the patterns are constructed 
	by considering and planning how these independent lights interact for the viewer.

    So this data for output pin 3 
		{ 3, 0, 0, {{MB_HIGH, 25, 0}, {MB_LOW, 25, 0}, {MB_LOOP, LOOP_PARAM(0, 3}, {MB_LOW, 300, 0}}, 0 },

    does this:
	State 0 is {MB_HIGH, 25, 0}             - LED on for 25 ms
	State 1 is {MB_LOW, 25, 0}              - LED off for 25 ms
	State 2 is {MB_LOOP, LOOP_PARAM(0, 3)}  - Loop back to state 0 and repeat it 2 more time (3 total)
	State 3 is {MB_LOW, 300, 0}             - LED off for 300 ms - this is a delay while the 'other' LED blinks

	So if we want to have a paired LED that works in tandem with this one and is blinking
	while this one is off, we need to implement the logic

	State 0 is {MB_LOW, 300, 0}             - LED off for 300 ms - this is a delay while the 'other' LED blinks
	State 1 is {MB_HIGH, 25, 0}             - LED on for 25 ms
	State 2 is {MB_LOW, 25, 0}              - LED off for 25 ms
	State 3 is {MB_LOOP, LOOP_PARAM(1, 3)}  - Loop back to state 1 and repeat it 2 more time (3 total)

	which provides the following table entry for output pin 4:
		{ 4, 0, 0, {{MB_LOW, 300, 0}, {MB_HIGH, 25, 0}, {MB_LOW, 25, 0}, {MB_LOOP, LOOP_PARAM(1, 3)}}, 0 },

	So if we look at them as a pair
		{ 3, 0, 0, {{MB_HIGH, 25, 0}, {MB_LOW, 25, 0}, {MB_LOOP, LOOP_PARAM(0, 3)}, {MB_LOW, 300, 0}}, 0 },
		{ 4, 0, 0, {{MB_LOW, 300, 0}, {MB_HIGH, 25, 0}, {MB_LOW, 25, 0}, {MB_LOOP, LOOP_PARAM(1, 3)}}, 0 },

	Similarly, more complex patterns and interactions can be planned and configured 
	into the data tables.
*/


#include "Multi_Blink3.h"  // type definitions

#define  BUTTON_PIN  7  // switch between patterns using this input

// Blink Table T - Modify this table to suit whatever the output requirements are 
// Add or delete lines as required to achieve the desired effects.
// Have multiple tables and switch between them to create different effects based on external inputs
// To add additional states the structure in the header file needs to be modified
//
ledTable  T1[] =
//Pin  St WUp  State 0              State 1              etc
{ 
  { 3, 0, 0, {{MB_ON, 50, 0, 0},   {MB_OFF, 100, 0 ,0}, {MB_LOOP, 0, LOOP_PARAM(0, 4), 0},  {MB_OFF, 800, 0, 0}, {MB_NULL, 0, 0, 0}}  },
  { 4, 0, 0, {{MB_OFF, 100, 0, 0}, {MB_ON, 50, 0, 0},   {MB_LOOP, 0, LOOP_PARAM(0, 4), 0},  {MB_OFF, 800, 0, 0}, {MB_NULL, 0, 0, 0}}  },
  { 5, 0, 0, {{MB_OFF, 800, 0, 0}, {MB_ON, 50, 0, 0},   {MB_OFF, 100, 0, 0}, {MB_LOOP, 0, LOOP_PARAM(1, 4), 0},  {MB_NULL, 0, 0, 0}}  },
  { 6, 0, 0, {{MB_OFF, 800, 0, 0}, {MB_OFF, 100, 0, 0}, {MB_ON, 50, 0, 0},   {MB_LOOP, 0, LOOP_PARAM(1, 4), 0},  {MB_NULL, 0, 0, 0}}  },
};
ledTable T2[] = 
{
  { 3, 0, 0, {{MB_ON, 20, 0, 0},   {MB_OFF, 50, 0, 0},  {MB_LOOP, 0, LOOP_PARAM(0, 3), 0},  {MB_ON, 250, 0, 0},  {MB_OFF, 450, 0, 0}} },
  { 4, 0, 0, {{MB_OFF, 450, 0, 0}, {MB_ON, 20, 0, 0},   {MB_OFF, 50, 0, 0},  {MB_LOOP, 0, LOOP_PARAM(1, 3), 0},  {MB_ON, 250, 0, 0}}  },
  
  { 5, 0, 0, {{MB_FADE_ON, 45, FADE_PARAM(1, 30), 0}, {MB_ON, 70, 0, 0}, {MB_FADE_OFF, 45, FADE_PARAM(30, 1), 0}, {MB_NULL,0,0,0}, {MB_NULL,0,0,0}} },
  { 6, 0, 0, {{MB_FADE_ON, 10, FADE_PARAM(0, 255), 0}, {MB_ON, 70, 0, 0}, {MB_FADE_OFF, 10, FADE_PARAM(255, 0), 0}, {MB_NULL,0,0,0}, {MB_NULL,0,0,0}} },
};



void BlinkInit(ledTable *pT, uint8_t tableSize)
{
  for (uint8_t i=0; i < tableSize; i++, pT++)
  {
    pinMode(pT->ledPin, OUTPUT);
    
    pT->nextWakeup = 0;
    digitalWrite(pT->ledPin, LOW);
    for (uint8_t j=0; j<MAX_STATE; j++)
    {
      pT->state[j].counter = CTR_UNDEF;
    }
  }
  return;
}


void MultiBlink(ledTable *pT, uint8_t tableSize)
// Finite state machine that uses the data passed to it to run the show
{
  for (int i=0; i < tableSize; i++, pT++)
  {
      uint8_t  cs = pT->currentState;  // current state shortcut

    // check if the state active time has expired (ie, it is less than current time)
    if (millis() >= pT->nextWakeup)
    {
      pT->nextWakeup = millis() + pT->state[cs].activeTime;

      switch (pT->state[cs].stateID)
      {
        case MB_OFF:
        case MB_ON:    // Write digital value
        {
          digitalWrite(pT->ledPin, pT->state[cs].stateID == MB_ON ? HIGH : LOW);
          pT->currentState = (pT->currentState + 1) % MAX_STATE;
        }
        break;
          
        case MB_FADE_ON:
        {
          // first time in this state? Check the counter
          if (pT->state[cs].counter == CTR_UNDEF)
          {
            pT->state[cs].counter = FADE_START_GET(pT->state[cs].data);
          }

          analogWrite(pT->ledPin, pT->state[cs].counter++);
          
          if (pT->state[cs].counter >= FADE_END_GET(pT->state[cs].data))
          {
            pT->state[cs].counter = CTR_UNDEF; // set up loop counter
            pT->currentState = (pT->currentState + 1) % MAX_STATE;
          }
        }
        break;

        case MB_FADE_OFF:
        {
          // first time in this state? Check the counter
          if (pT->state[cs].counter == CTR_UNDEF)
          {
            pT->state[cs].counter = FADE_START_GET(pT->state[cs].data);
          }

          analogWrite(pT->ledPin, pT->state[cs].counter--);
          
          if (pT->state[cs].counter <= FADE_END_GET(pT->state[cs].data))
          {
            pT->state[cs].counter = CTR_UNDEF; // set up loop counter
            pT->currentState = (pT->currentState + 1) % MAX_STATE;
          }
        }
        break;

        case MB_LOOP:  // loop back to specified state if there is still count left
        {
          // first time in this state? Check the counter
          if (pT->state[cs].counter == CTR_UNDEF)
          {
            pT->state[cs].counter = LOOP_SP_GET(pT->state[cs].data);
          }

          // loop around or keep going?          
          if (pT->state[cs].counter-- > 0)
          {
            pT->currentState = LOOP_STATE_GET(pT->state[cs].data);
            pT->nextWakeup = 0;  // do it immediately
          }
          else 
          {
            pT->state[cs].counter = CTR_UNDEF; // set up loop counter
            pT->currentState = (pT->currentState + 1) % MAX_STATE;
          }
        }
        break;  
          
        default:  // just move on - handles NULL and any stupid states we may get into
        {
          pT->currentState = (pT->currentState + 1) % MAX_STATE;
        }
        break;
      }
    }
  }
  
  return;
}

void setup(void)
{
  BlinkInit(T1, ARRAY_SIZE(T1));
  BlinkInit(T2, ARRAY_SIZE(T2));
}


void loop(void)
{
  if (digitalRead(BUTTON_PIN) == LOW)
  {
    MultiBlink(T1, ARRAY_SIZE(T1));
  }
  else
  {
    MultiBlink(T2, ARRAY_SIZE(T2));
  }    
}

