// Multi_Blink2
//
// Blink lots of LEDs at different frequencies simultaneously, include delays and loops in patterns
//
// Marco Colli - September 2012
//
// Demonstrates the way to carry out multiple time based tasks without using the delay() function
// Demonstrates the use of structures (and structures within structures)
// Demonstrates a data driven approach to programming to create compact, reusable code
//

#include "Multi_Blink2.h"  // type definitions

// Blink Table T - Modify this table to suit whatever the output requirements are 
// Add or delete lines as required to achieve the desired effects.
// To add additional states the structure in the header file needs to be modified
//
ledTable  T[] =
//Pin  St Loop  State 0          State 1            State 2          State 3         Wkup
{ 
  { 3, 0, 0, {{MB_HIGH, 50, 0}, {MB_LOW, 100, 0}, {MB_LOOP, 4, 0}, {MB_LOW, 800, 0}}, 0 },
  { 4, 0, 0, {{MB_LOW, 100, 0}, {MB_HIGH, 50, 0}, {MB_LOOP, 4, 0}, {MB_LOW, 800, 0}}, 0 },
  { 5, 0, 0, {{MB_LOW, 800, 0}, {MB_HIGH, 50, 0}, {MB_LOW, 100, 0}, {MB_LOOP, 4, 1}}, 0 },
  { 6, 0, 0, {{MB_LOW, 800, 0}, {MB_LOW, 100, 0}, {MB_HIGH, 50, 0}, {MB_LOOP, 4, 1}}, 0 },
};

// Self adjusting constants for loop indexes
#define  MAX_STATE  (sizeof(T[0].state)/sizeof(stateDef))
#define  MAX_LED    (sizeof(T)/sizeof(ledTable))

void setup()
{
  for (int i=0; i < MAX_LED; i++)
  {
    pinMode(T[i].ledPin, OUTPUT);
    
    T[i].nextWakeup = 0;
    digitalWrite(T[i].ledPin, LOW);
    T[i].currentLoop = LOOP_UNDEF;
  }
}

void loop()
{
  for (int i=0; i < MAX_LED; i++)
  {
    // check if the state active time has expired (ie, it is less than current time)
    if (millis() >= T[i].nextWakeup)
    {
      T[i].nextWakeup = millis() + T[i].state[T[i].currentState].activeTime;

      switch (T[i].state[T[i].currentState].activeVal)
      {
        case MB_LOW:
        case MB_HIGH:    // Write digital value
          digitalWrite(T[i].ledPin, T[i].state[T[i].currentState].activeVal == MB_HIGH ? HIGH : LOW);
          T[i].currentState = (++T[i].currentState) % MAX_STATE;
          break;
          
        case MB_LOOP:  // loop back to specified state if there is still count left
          // first time in this state set the loop counter
          if (T[i].currentLoop == LOOP_UNDEF)
          {
            T[i].currentLoop = T[i].state[T[i].currentState].activeTime;
          }

          // loop around or keep going?          
          if (T[i].currentLoop-- > 0)
          {
            T[i].currentState = T[i].state[T[i].currentState].nextState;
            T[i].nextWakeup = 0;  // do it immediately
          }
          else 
          {
            T[i].currentLoop = LOOP_UNDEF; // set up loop counter
            T[i].currentState = (++T[i].currentState) % MAX_STATE;
          }
          break;  
          
        default:  // just move on - handles NULL and any stupid states we may get into
          T[i].currentState = (++T[i].currentState) % MAX_STATE;
          break;
      }
    }
  }
}
