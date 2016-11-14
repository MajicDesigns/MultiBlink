// Multi_Blink
//
// Blink lots of LEDs at different frequencies simultaneously
//
// Marco Colli - May 2012
//
// Demonstrates the way to carry out multiple time based tasks without using the delay() function
// Demonstrates the use of structures (and structures within structures)
// Demonstrates a data driven approach to programming to create compact, reusable code
//

#include "Multi_Blink1.h"  // type definitions

// Blink Table T - Modify this table to suit whatever the output requirements are 
// Add or delete lines as required to achieve the desired effects.
//
ledTable  T[] =
//Pin  St   State 0      State 1  LastTime
{ 
  { 3, 0, {{HIGH, 300}, {LOW, 300}}, 0 },
  { 4, 1, {{HIGH, 300}, {LOW, 600}}, 0 },
  { 5, 0, {{HIGH, 500}, {LOW, 500}}, 0 },
  { 6, 1, {{HIGH,  50}, {LOW, 100}}, 0 },
  { 7, 0, {{HIGH, 100}, {LOW,  50}}, 0 },
  { 8, 1, {{HIGH, 500}, {LOW, 500}}, 0 },
  { 9, 0, {{HIGH, 400}, {LOW, 600}}, 0 },
  {10, 0, {{HIGH, 600}, {LOW, 400}}, 0 },
};

// Self adjusting constants for loop indexes
#define  MAX_STATE  (sizeof(T[0].state)/sizeof(stateDef))
#define  MAX_LED    (sizeof(T)/sizeof(ledTable))

void setup()
{
  for (int i=0; i < MAX_LED; i++)
  {
    pinMode(T[i].ledPin, OUTPUT);
    
    T[i].lastTransTime = millis();
    digitalWrite(T[i].ledPin, T[i].state[T[i].currentState].activeVal);
  }
}

void loop()
{
  for (int i=0; i < MAX_LED; i++)
  {
    // check if the state active time has expired (ie, it is less than current time)
    if (millis() >= T[i].lastTransTime + T[i].state[T[i].currentState].activeTime)
    {
      // switch to the next state with wrapround
      T[i].currentState = (T[i].currentState + 1) % MAX_STATE;
      
      // write out the next state value
      T[i].lastTransTime = millis();
      digitalWrite(T[i].ledPin, T[i].state[T[i].currentState].activeVal);
    }
  }
}

