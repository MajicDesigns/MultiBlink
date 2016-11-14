// Multi_Blink2.h
//
// Blink lots of LEDs at diffreent frequencies simultaneously
//
// Header file is required to be able to define the structured types
//
#include <Arduino.h>

#ifndef  MULTIBLINKH
#define  MULTIBLINKH

// State values for FSM
#define  MB_NULL  0
#define  MB_LOW   1
#define  MB_HIGH  2
#define  MB_LOOP  3

#define  LOOP_UNDEF  255

typedef struct
{
  uint8_t  activeVal;     // state value for this state to be active (MB_* defines)
  uint16_t activeTime;    // time to stay active in this state stay in milliseconds or loop counter 
  uint8_t  nextState;     // If MB_LOOP this is the state to loop back/forward to 
} stateDef;

typedef struct
{
  uint8_t  ledPin;         // Arduino I/O pin number
  uint8_t  currentState;   // current active state
  uint8_t  currentLoop;    // current position in the cycle
  stateDef state[4];       // the MB_* state definitions. Add more states if required
  uint32_t nextWakeup;     // the 'time' for the next wakeup - saves the millis() value
} ledTable;

#endif