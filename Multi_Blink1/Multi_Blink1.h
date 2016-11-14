// Multi_Blink.h
//
// Blink lots of LEDs at diffreent frequencies simultaneously
//
// Header file is required to be able to define the structured types
//
#include <Arduino.h>

#ifndef  MULTIBLINKH
#define  MULTIBLINKH

typedef struct
{
  uint8_t  activeVal;     // digital value for this state to be active (HIGH/LOW)
  uint16_t activeTime;    // time to stay active in this state stay in milliseconds 
} stateDef;

typedef struct
{
  uint8_t  ledPin;         // Arduino I/O pin number
  uint8_t  currentState;   // current active state
  stateDef state[2];       // the ON and OFF state definitions. Add more states if required
  uint32_t lastTransTime;  // the 'time' of the last state transition - saves the millis() value
} ledTable;

#endif

