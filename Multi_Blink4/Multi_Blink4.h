/*
 Multi_Blink4.h

 Blink lots of LEDs at different frequencies simultaneously

 Header file is required to be able to define the structured types
*/
#include <Arduino.h>

#ifndef  MULTIBLINKH
#define  MULTIBLINKH

// State values for the Finit State Machine (FSM)
#define  MB_NULL     0
#define  MB_ON       1
#define  MB_OFF      2
#define  MB_FADE_ON  3
#define  MB_FADE_OFF 4    // stateDef.data is the start and end PWM Setting - use FADE_* macros
#define  MB_LOOP     5    // stateDef.data is the next state position and counter setpoint, use LOOP_* macros
#define	 MB_GOTO	 6
#define	 MB_STOP	 9

// Special counter initialisation value (undefined)
#define  CTR_UNDEF  255

// The number of state definitions for each element of the LED table.
// There are up to MAX_STATE transitions allowed per element of the LED table. If more than the
// number defined are required, change this constant. All the table entries will get MAX_STATE
// transitions, even if they are not used, so big numbers use up lots of memory!
#define	 MAX_STATE	5

#define	ARRAY_SIZE(x)	(sizeof(x)/sizeof(x[0]))

// Macros used to pack and unpack 8 bit values into a 16 bit data word
#define  FADE_PARAM(s, e)    (((s) << 8) | (e))
#define  FADE_START_GET(n)   highByte(n)
#define  FADE_END_GET(n)     lowByte(n)

#define  LOOP_PARAM(s, c)    (((s) << 8) | (c))
#define  LOOP_STATE_GET(n)   highByte(n)
#define  LOOP_SP_GET(n)      lowByte(n)

// Data structures for the state definition and the LED table
typedef struct
{
  uint8_t  stateID;       // state value for this state to be active (MB_* defines)
  uint16_t activeTime;    // time to stay active in this state in milliseconds
  uint16_t data;          // data store for state context information/parameters (depends on MB_*)
  uint8_t  counter;       // generic counter for the state context information. CTR_UNDEF is special value.
} stateDef;

typedef struct
{
  uint8_t  ledPin;         // Arduino I/O pin number
  uint8_t  currentState;   // current active state
  uint32_t nextWakeup;     // the 'time' for the next wakeup - saves the millis() value
  stateDef state[MAX_STATE];  // the MB_* state definitions. Add more states if required
} ledTable;

#endif

