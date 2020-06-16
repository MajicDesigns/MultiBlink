/*
 Multi_Blink5_Neo

 Blink lots of Neopixel LEDs at different frequencies simultaneously, include fades, 
 delays and loops in patterns.

 Marco Colli - June 2020
 
 Demonstrates 
  - the way to carry out multiple time based tasks without using the delay() function
  - the use of structures (and unions and structures within structures)
  - a data driven approach to programming to create compact, reusable code

 How this works
 ==============
   This sketch separates the HOW (code) and the WHAT (data) of multi-tasking 
   the blinking of LEDs. How we blink LEDS is the same no matter what LED we
   are blinking, so this can be defined independently. Which LEDs the blinking 
   applies to and how long it lasts is separately defined and applied to the 
   code. This data driven approach is applicable to any number of situations 
   where repeated independent actions need to be carried out simultaneously.

   The code used in this sketch implements a general approach to manipulating 
   the neopixel animations based on a Finite State Machine. What to do in each 
   state of the FSM is defined using the constants MB_*, arranged in data tables 
   that are 'run' by the overall FSM. To change the behavior of the software, 
   the data table is modified for the new requirements/patterns.

   The sequeunce tables (defined in T[] below) consist of an arbitrary length
   array of sequeunce_t structures that define the parameters for the neopixels
   and 'remember' the current FSM parameters - for example, its current state
   and how long it should last.

   Each sequence_t entry has associated with it up to MAX_STATE states (defined 
   using stateDef_t structures) that specify what the FSM will do when it is in 
   that state. To change the number of allowable states, modify the MAX_STATE 
   constant.

   The tables can be statically or dynamically initialized. In this code the 
   patterns are defined statically in the T[] tables.

   The overall effect is that the sequence tables work like a macro programming
   language to direct the how the LEDs should be animated.

 Defining stateDef entries
 =========================
   MB_SET
   ------
   Keep the LED on the specified color for the specified time. 

   The macro SET(t, c) is used to statically initialize the set parameters, 
   where t is the time to keep the color c on.

   For example, the entry
   { SET(25, CRGB(255,255,255) } 
   keeps the color output at the RGB color (255,255,255) for 25 milliseconds.

   MB_LOOP
   -------
   Makes the pattern loop back to the nominated array element in the pattern 
   for the specified number of times after a specified delay. 

   The macro DO(t, j, n) is used to set the loop parameters, where t is the
   delay time before testing the loop, j is the jump-to array index (zero 
   based) to loop back to and n is the total number of times to execute the 
   loop. A table entry will loop back to the start when it reaches the end 
   or use MB_GOTO for infinite loops.

   For example, the entry
   { DO(10, 0, 3) }
   causes the FSM to delay 10 milliseconds and then loop back to element 0.
   The process is repeated it 2 more time (3 total).

   MB_FADE
   -------
   Fades a led from a starting color to an end color (both FastLED CRGB values)
   over a specified time period. This is, in effect, a special kind of loop that 
   interpolates between two colors.

   The macro FADE(t, c1, c2) is used to initialize the fade effect parameters in
   the data structure, where t is the overall time to fade, c1 and c2 are the start
   and end color CRGB values, respectively.

   For example, the entry 
   { FADE(500, CRGB(0,0,0), CRGB(255, 255, 255) }
   fades a led between CRGB colors (0,0,0) and (255,255,255) over 500 milliseconds.

   MB_GOTO
   -------
   Go to the state specified as the next state, with an optional time delay 
   before jumping to the new state.

   The macro GOTO(t, j) is used to initialize the goto parameters in the data 
   structure, where t is the time before jumping to the zero-based jump-to state
   index j.
    
   For example, the entry
   { GOTO(55, 2) }
   goes to state 2 after 55 milliseconds have elapsed.

   MB_STOP
   -------
   Stop executing the pattern. It disables a pattern from running by changing
   the sequence enable flag to false.

   The macro STOP is used to create a MB_STOP state entry.

   MB_NULL
   -------
   Placemarker for sequence entries that are unused. These entries are skipped.

   The macro NOP (no operation) is used to create a MB_NULL entry.

 Building a ledTable
 ===================
   Whilst ledTable entries are defined and run independently, the patterns are constructed 
   by considering and planning how these independent lights interact for the viewer.

   So this data for led index 3 
   { 3, {SET(25, CRGB(0,0,0), SET(25, CRGB(255,255,255), DO(0, 0, 3), SET(300, CRGB(0,0,0)} },

   does this:
   State 0 is LED at RGB BLACK for 25 ms
   State 1 is LED at RGB WHITE for 25 ms
   State 2 is loop back to state 0 and repeat it 2 more time (3 total)
   State 3 is LED at RGB BLACK for 300 ms - this is a delay while the 'other' LED blinks (see below)

   So if we want to have a paired LED that works in tandem with this one and is blinking
   while this one is off, we need to implement the logic

   State 0 is LED at RGB BLACK for 300 ms - this is a delay while the 'other' LED blinks (see above)
   State 1 is LED at RGB WHITE for 25 ms
   State 2 is LED at RGB BLACK for 25 ms
   State 3 is loop back to state 1 and repeat it 2 more time (3 total)

   which provides the following table entry for output led 4
   { 4, {SET(300, CRGB(0,0,0), SET(25, CRGB(255,255,255), SET(25, CRGB(0,0,0), DO(0, 0, 3)} },

   So if we look at them as a pair
   { 3, {SET(25, CRGB(0,0,0), SET(25, CRGB(255,255,255), DO(0, 0, 3), SET(300, CRGB(0,0,0)} },
   { 4, {SET(300, CRGB(0,0,0), SET(25, CRGB(255,255,255), SET(25, CRGB(0,0,0), DO(0, 0, 3)} },

   Similarly, more complex patterns and interactions can be planned and configured 
   into the data tables.
*/
#include <FastLED.h>

#ifndef DEBUG
#define DEBUG 0   // enable/disable debug output
#endif

#if DEBUG
#define PRINT(s, v)   { Serial.print(F(s)); Serial.print(v); }
#define PRINTX(s, v)  { Serial.print(F(s)); Serial.print(F("0x")); Serial.print(v, HEX); }
#define PRINTS(s)     { Serial.print (F(s)); }
#define PRINTSTATE(s, l, i) \
{ \
  Serial.print(F("\n")); Serial.print(now); \
  Serial.print(F(" ")); Serial.print(F(s)); \
  Serial.print(F(" L:")); Serial.print(l); \
  Serial.print(F("[")); Serial.print(cs); \
  Serial.print(F("]")); \
}
#else
#define PRINT(s, v)
#define PRINTX(s, v)
#define PRINTS(s)
#define PRINTSTATE(s, l, i)
#endif

const uint8_t SWITCH_TABLE = 4;  // switch between patterns using this input

// FastLED --------------
// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
const uint8_t LED_DATA_PIN = 2;
// const uint8_t CLOCK_PIN = 13;

const uint8_t NUM_LEDS = 8;     // define the number of LEDs in the LED string

#define LED_TYPE    WS2812      // FastLED - LED type
#define COLOR_ORDER GRB         // FastLED - RGB color order

CRGB leds[NUM_LEDS];            // FastLED - define the array of leds
bool needsUpdate;               // true when a color has changed

// State values for the Finite State Machine (FSM)
typedef enum { MB_NULL, MB_SET, MB_FADE, MB_LOOP, MB_GOTO, MB_STOP} state_t;

// Class to encapsulate fading a neopixel using Bresenham's algorithm 
// for a straight line in 3D (RGB color) space.
// Algorithm implementation adapted from http://members.chello.at/easyfilter/bresenham.html
class cNeoFade
{
public:

  cNeoFade(void) { _ended = true; }

  inline bool isEnded(void) { return(_ended); }

  void setFade(uint16_t period, const CRGB &rgb0, const CRGB &rgb1)
  // initialize for a fade between rgb0 and rgb1
  {
    _r0 = rgb0.r;  _r1 = rgb1.r;
    _g0 = rgb0.g;  _g1 = rgb1.g;
    _b0 = rgb0.b;  _b1 = rgb1.b;

    _dR = uiabs(_r1, _r0);  _sR = _r0 < _r1 ? 1 : -1;
    _dG = uiabs(_g1, _g0);  _sG = _g0 < _g1 ? 1 : -1;
    _dB = uiabs(_b1, _b0);  _sB = _b0 < _b1 ? 1 : -1;
    _dMax = max3(_dR, _dG, _dB);
    _steps = _dMax;
    _r1 = _g1 = _b1 = _dMax / 2;            /* error offset */

    // step timing
    _timeInterval = period / _steps;
    if (_timeInterval == 0) _timeInterval = 1;
    _timeStart = 0;

    _ended = false;
  }

  bool getNext(uint32_t nowTime, CRGB *rgb)
  // Get the next coordinate on the fade 'line'
  // Return true if the coordinate returned is valid
  {
    if (!_ended && nowTime - _timeStart < _timeInterval)
      return(false);

    _timeStart = millis();

    rgb->r = _r0; rgb->g = _g0; rgb->b = _b0;

    _ended = (_steps == 0);
    if (!_ended)
    {
      _steps--;
      _r1 -= _dR; if (_r1 < 0) { _r1 += _dMax; _r0 += _sR; }
      _g1 -= _dG; if (_g1 < 0) { _g1 += _dMax; _g0 += _sG; }
      _b1 -= _dB; if (_b1 < 0) { _b1 += _dMax; _b0 += _sB; }
    }

    return(!_ended);
  }

private:
  int16_t _r0, _g0, _b0, _r1, _g1, _b1, _steps;
  int16_t _dR, _dG, _dB, _dMax;
  int8_t _sR, _sG, _sB;
  bool _ended;

  // timing parameters
  uint32_t _timeStart; // animation time start
  uint32_t _timeInterval; // interval between color changes

  inline uint16_t uiabs(uint16_t a, uint16_t b) { return((a > b) ? (a - b) : (b - a)); }

  uint16_t max3(uint16_t x, uint16_t y, uint16_t z)
  // find and return the maximum of 3 values
  {
    uint16_t m = z;    // assume z to start with

    if (x > y) { if (x > z) m = x; }
    else { if (y > z) m = y; }

    return(m);
  }
};

// The number of state definitions for each element of the LED table.
// There are up to MAX_STATE transitions allowed per element of the LED table.
// If more than the number defined are required, change this constant.
// NOTE: All the table entries will get MAX_STATE transitions allocated, even
// if they are not used, so big numbers use up lots of memory!
const uint8_t MAX_STATE = 5;

#define  ARRAY_SIZE(a)  (sizeof(a)/sizeof((a)[0]))

// Data structures for the state definition and the LED table
typedef struct
{
  state_t  stateID;       // state value for this state (MB_*)
  uint32_t activeTime;    // active time for this state

  // various state presets
  uint32_t data1;        // the color for this on/off state or start of fade
  uint32_t data2;        // end color fade
} stateDef_t;

typedef struct
{
  uint8_t ledId;          // led number in the LED array
  stateDef_t state[MAX_STATE];  // the MB_* state definitions. Add more states if required
} sequence_t;

typedef struct
{
  // sequence variables
  bool       enabled;        // true if this table entry is enabled
  uint8_t    currentState;   // index of currently active state
  uint32_t   lastWakeup;     // saves the millis() value when last woken up

  // state variables
  cNeoFade* nf;      // neofade object to control fading execution
  uint16_t counter;  // current loop counter
} scratchpad_t;

// Macros used to statically initialize the data
#define  SET(t, c)        {MB_SET,(t),(c),0}
#define  DO(t, j, n)      {MB_LOOP,(t),(j),(n)}
#define  FADE(t, c1, c2)  {MB_FADE,(t),(c1),(c2)}
#define  GOTO(t, j)       {MB_GOTO,(t),(j),0}
#define  STOP             {MB_STOP,0,0,0}
#define  NOP              {MB_NULL,0,0,0}

// Sequence Tables - Modify these tables to suit whatever the output 
// requirements are.
// Add or delete lines as required to achieve the desired effects.
// Have multiple tables and switch between them to create different effects 
// based on external inputs or use.
const sequence_t PROGMEM T1[] =
{
  // Alternate double flashers, like emergency vehicles
  { 0, {SET(100, CRGB::Blue), SET(50, CRGB::Red), DO(0, 0, 4), SET(600, CRGB::Black), NOP} },
  { 1, {SET(50, CRGB::Red), SET(100, CRGB::Blue), DO(0, 0, 4), SET(600, CRGB::Black), NOP} },
  { 2, {SET(600, CRGB::Black), SET(50, CRGB::Red), SET(100, CRGB::Blue), DO(0, 1, 4), NOP} },
  { 3, {SET(600, CRGB::Black), SET(100, CRGB::Blue), SET(50, CRGB::Red), DO(0, 1, 4), NOP} },
  { 4, {SET(600, CRGB::Black), SET(100, CRGB::Blue), SET(50, CRGB::Red), DO(0, 1, 4), NOP} },
  { 5, {SET(600, CRGB::Black), SET(50, CRGB::Red), SET(100, CRGB::Blue), DO(0, 1, 4), NOP} },
  { 6, {SET(50, CRGB::Red), SET(100, CRGB::Blue), DO(0, 0, 4), SET(600, CRGB::Black), NOP} },
  { 7, {SET(100, CRGB::Blue), SET(50, CRGB::Red), DO(0, 0, 4), SET(600, CRGB::Black), NOP} },
};
scratchpad_t S1[ARRAY_SIZE(T1)];

const sequence_t PROGMEM T2[] =
{
  // Aircraft wing tip flashers
  { 0, {SET(20, CRGB::White), SET(50, CRGB::Black), DO(0, 0, 3), SET(250, CRGB::White), SET(530, CRGB::Black)} },
  { 1, {SET(530, CRGB::Black), SET(20, CRGB::White), SET(50, CRGB::Black), DO(0, 1, 3), SET(250, CRGB::White)} },
  // Simple fade in/out 5 times and then stop
  { 3, {FADE(2048, CRGB::Black, CRGB::SeaGreen), FADE(2048, CRGB::SeaGreen, CRGB::Black), DO(1000, 0, 4), STOP, NOP} },
  // Revolving lighthouse beam
  { 4, {FADE(1400, CRGB::Black, CRGB::Beige), SET(100, CRGB::Beige), FADE(1400, CRGB::Beige, CRGB::Black), NOP, NOP} },
  // Railway crossing alternating red
  { 6, {SET(1000, CRGB::Black), SET(1000, CRGB::Red), NOP, NOP, NOP} },
  { 7, {SET(1000, CRGB::Red), SET(1000, CRGB::Black), NOP, NOP, NOP} },
};
scratchpad_t S2[ARRAY_SIZE(T2)];

const uint16_t T_WAIT = 200;
const uint16_t T_FADE = (NUM_LEDS/2)*T_WAIT;
const uint32_t COL_ON = (uint32_t)CRGB::Yellow;
const uint32_t COL_OFF = (uint32_t)CRGB::Black;
const sequence_t PROGMEM T3[] =
{
  // Fading LED chaser
  { 0, {GOTO(T_WAIT * 0, 1), FADE(T_FADE, COL_ON, COL_OFF), GOTO(T_WAIT * 7, 0), NOP, NOP} },
  { 1, {GOTO(T_WAIT * 1, 1), FADE(T_FADE, COL_ON, COL_OFF), GOTO(T_WAIT * 6, 0), NOP, NOP} },
  { 2, {GOTO(T_WAIT * 2, 1), FADE(T_FADE, COL_ON, COL_OFF), GOTO(T_WAIT * 5, 0), NOP, NOP} },
  { 3, {GOTO(T_WAIT * 3, 1), FADE(T_FADE, COL_ON, COL_OFF), GOTO(T_WAIT * 4, 0), NOP, NOP} },
  { 4, {GOTO(T_WAIT * 4, 1), FADE(T_FADE, COL_ON, COL_OFF), GOTO(T_WAIT * 3, 0), NOP, NOP} },
  { 5, {GOTO(T_WAIT * 5, 1), FADE(T_FADE, COL_ON, COL_OFF), GOTO(T_WAIT * 2, 0), NOP, NOP} },
  { 6, {GOTO(T_WAIT * 6, 1), FADE(T_FADE, COL_ON, COL_OFF), GOTO(T_WAIT * 1, 0), NOP, NOP} },
  { 7, {GOTO(T_WAIT * 7, 1), FADE(T_FADE, COL_ON, COL_OFF), GOTO(T_WAIT * 0, 0), NOP, NOP} },
};
scratchpad_t S3[ARRAY_SIZE(T3)];

void LEDInit(void)
{
  for (uint8_t i = 0; i < NUM_LEDS; i++)
    leds[i] = CRGB::Black;
  needsUpdate = true;
}

void BlinkInit(scratchpad_t *pS, const uint8_t tableSize)
// Initialize one sequence's scratchpad data
{
  uint32_t now = millis();

  for (uint8_t i=0; i < tableSize; i++, pS++)
  {
    pS->enabled = true;
    pS->currentState = 0;
    pS->lastWakeup = now;
    pS->counter = 0;
    if (pS->nf != nullptr)
    {
      delete pS->nf;
      pS->nf = nullptr;
    }
  }
  needsUpdate = true;
}

void MultiBlink(const sequence_t *pT, scratchpad_t *pS, const uint8_t tableSize)
// Finite state machine that uses the data passed to it to run the show
{
  uint32_t now = millis();      // keep the same time base for all LEDs
  
  for (uint8_t i=0; i < tableSize; i++, pT++, pS++)
  {
    if (pS->enabled)
    {
      uint8_t  cs = pS->currentState;  // current state shortcut variable
      bool timeExpired;
      bool setNextState = false;  // flag to move the next in sequence
      int16_t nextState = -1;     // holds the next state index number for jumps
      sequence_t T;

      // make a local RAM copy of sequence table entry
      memcpy_P(&T, pT, sizeof(T));

/*
      //   Deep debugging here - too much text for normal tracing
      PRINT("\nIX:", i);
      PRINT(" L:", T.ledId);
      PRINT(" sIX:", pS->currentState);
      PRINT(" ID:", T.state[cs].stateID);
      PRINT(" T:", T.state[cs].activeTime);
      PRINT(" D:[", T.state[cs].data1);
      PRINT(",", T.state[cs].data2);
      PRINTS("]");
*/
      // check timers expiration
      timeExpired = (now - pS->lastWakeup >= T.state[cs].activeTime);
      if (timeExpired) PRINTSTATE(">> TIME ", T.ledId, cs);

      switch (T.state[cs].stateID)
      {
      case MB_SET:   // set a led color
        setNextState = timeExpired;
        // avoid FastLED updates if we don't need them
        // (ie, set the color once only)
        if (leds[T.ledId] != CRGB(T.state[cs].data1))
        {
          PRINTSTATE("> SET ", T.ledId, cs);
          PRINTX(" RGB:", T.state[cs].data1);
          leds[T.ledId] = CRGB(T.state[cs].data1);
          needsUpdate = true;
        }
        break;

      case MB_FADE:  // transition fade between colors
        setNextState = timeExpired;

        if (setNextState)   // this is the last step in the fade sequence
        {
          PRINTSTATE("> FADE E", T.ledId, cs);
          PRINT(" T:", now - pS->lastWakeup);
          PRINT("/", T.state[cs].activeTime);
          PRINTS("ms");

          // clean up the temporary fade object ...
          if (pS->nf != nullptr) delete pS->nf;
          pS->nf = nullptr;

          // ... and definitely set the final color!
          // This is needed because there may be a mismatch between aplication
          // and cNeoFade class timers due to integer rounding errors when calculating 
          // (period/steps). We want to give the main application timer priority, so
          // if the whole transition is not completed we need to ensure the final 
          // color is explicitly set here.
          leds[T.ledId] = CRGB(T.state[cs].data2);
          needsUpdate = true;
        }
        else
        {
          if (pS->nf == nullptr)
          {
            cNeoFade* nf = new cNeoFade;

            PRINTSTATE("> FADE S", T.ledId, cs);
            if (nf == nullptr) PRINTS(" !!! No memory allocated");

            // first time - set up the fade object and main timer
            pS->nf = nf;
            if (nf != nullptr)
              nf->setFade(T.state[cs].activeTime, CRGB(T.state[cs].data1), CRGB(T.state[cs].data2));
           
            // ... and set the initial color
            leds[T.ledId] = CRGB(T.state[cs].data1);
            needsUpdate = true;
          }
          else if (!pS->nf->isEnded())  // fade transition still happening
          {
            // ask for next coordinates and show if they have changed
            CRGB rgb;

            if (pS->nf->getNext(now, &rgb))
            {
              //PRINTSTATE("> FADE U", T.ledId, cs);
              //PRINT(" R:", rgb.r); PRINT(" G:", rgb.g); PRINT(" B:", rgb.b);
              leds[T.ledId] = rgb;
              needsUpdate = true;
            }
          }
        }
        break;

      case MB_LOOP:  // loop back to specified state if there is still count left
        if (timeExpired)
        {
          pS->counter++;    // just done one more loop

          PRINTSTATE("> LOOP", T.ledId, cs);

          // keep looping or push through to next sequential?          
          if (T.state[cs].data2 > pS->counter)
          {
            PRINT(" J:", T.state[cs].data1);
            // keep looping!
            nextState = T.state[cs].data1;
          }
          else
          {
            PRINTS(" end");
            // looping completed, reset counters for next execution
            pS->counter = 0;
            setNextState = true;
          }
        }
        break;

      case MB_GOTO:  // go to the specified sequence index (unconditional jump)
        // check if the delay time has expired
        if (timeExpired)
        {
          PRINTSTATE("> GOTO", T.ledId, cs);
          PRINT(" J:", T.state[cs].data1);
          nextState = T.state[cs].data1;
        }
        break;

      case MB_STOP:  // do nothing as we want this sequence to end with this entry
        PRINTSTATE("> STOP", T.ledId, cs);
        pS->enabled = false;
        break;

      case MB_NULL:  // nothing to do, skip this and keep going
        PRINTSTATE("> NULL", T.ledId, cs);
        setNextState = true;
        break;

      default:      // just note stupid error states and move on
        PRINTSTATE("> !!! UNKNOWN:", T.ledId, cs);
        setNextState = true;
        break;
      }

      // set next state if required
      if (setNextState)
        nextState = pS->currentState + 1;

      if (nextState != -1)
      {
        pS->lastWakeup += T.state[cs].activeTime;
        pS->currentState = nextState % MAX_STATE; // keep it in bounds
        PRINT(" S->", nextState);
      }
    } // if enabled
  } // for
}

bool switchState(void)
// Detect a rising edge from the switch
// return true when detected
{
  static bool  bLastHigh = true;
  bool  b = (digitalRead(SWITCH_TABLE) == HIGH);
  bool  bRet = !bLastHigh && b;

  bLastHigh = b;

  return(bRet);
}

void InitTables(void)
// Initialize all the LED tables
{
  LEDInit();
  BlinkInit(S1, ARRAY_SIZE(S1));
  BlinkInit(S2, ARRAY_SIZE(S2));
  BlinkInit(S3, ARRAY_SIZE(S3));
}

void setup(void)
{
#if DEBUG
  Serial.begin(57600);
#endif
  PRINTS("\n[MultiBlink5 Neo]");
  PRINT("\nSize of stateDef_t: ", sizeof(stateDef_t));
  PRINT("\nSize of sequence_t: ", sizeof(sequence_t));
  PRINT("\nSize of cNeoFade  : ", sizeof(cNeoFade));

  FastLED.addLeds<LED_TYPE, LED_DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(255);

  InitTables();
}

void loop(void)
{
  static uint8_t nTable = 0;

  // check if we need to switch table
  if (switchState())
  {
    nTable++;
    InitTables();
  }

  if (needsUpdate)  // this can be set in a few different places!
  {
    FastLED.show();
    needsUpdate = false;
  }

  // run with the selected table
  switch(nTable)
  {
  case 0:  MultiBlink(T1, S1, ARRAY_SIZE(T1));  break;
  case 1:  MultiBlink(T2, S2, ARRAY_SIZE(T2));  break;
  case 2:  MultiBlink(T3, S3, ARRAY_SIZE(T3));  break;
  default:  nTable = 0;
  }    
}
