/*
 * Name:	LightController.ino
 * Author:	User "benjaf" at plantedtank.net forums
 * URL:		https://github.com/benjaf/LightController
 */

#include <Wire.h>
#include <RTClib.h>

// ----------------------- Pins ----------------------- 
// PWM Pins
// Please note: Some PWM pins should not be used for lights -
// On the Uno these are pins 5 and 6. They do not behave like the others
// due to some shared timers!
#define L1Pin 9
#define L2Pin 10
// ... and so on

// ----------------------- Named Values ----------------------- 
#define CHANNELS 2		// Number of channels, change if more are required
#define MAXPERIODS 5	// Max number of light periods, change if more are required

// These are just used to make the lookup code more easily readable:
#define START_H 0
#define START_M 1
#define END_H 2
#define END_M 3
#define INTENSITY 4

// ----------------------- Variables ----------------------- 
// RTC
RTC_DS1307 RTC;

// Time
int Hours, Minutes, Seconds;

// ----------------------- Lights ----------------------- 
// Light period format: {HourStart, MinuteStart, HourEnd, MinuteEnd, Intensity}

// Gap between light periods is used to fade/increase intensity gradually
// Example: {0, 0, 8, 0, 0}, {8, 30, 11, 0, 255} ...
// Explanation: 
//  00:00 - 08:00 -> Light OFF
//  08:00 - 08:30 -> Increment light towards desired intensity
//  08:30 - 11:00 -> Light Fully ON

// Min intensity value: 0 (OFF)
// Max intensity value: 255 (Fully ON)
// First StartTime value MUST be 00:00
// SOME EndTime value MUST be 24:00

// Any row ending with 24:00 will be considered last.
// Not all periods must be filled with meaningfull values for each channel.
// Periods must always be chronological and without overlaps.
// There must always be rows defined for the max number of periods, but not all have to be valid
// Example: {{0, 0, 24, 0, 255},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0},{0, 0, 0, 0, 0}}
// Explanation: 
// Channel is ALWAYS FULLY ON. 
// As the first period never ends, the remaining rows are only there to fill out the necessary space. 

// Add more timing tables if more channels are added

int lightMatrix[CHANNELS][MAXPERIODS][5] = 
{
  {
    {0, 0, 8, 0, 0},
    {8, 30, 11, 0, 255},
    {11, 10, 13, 0, 0},
    {13, 10, 20, 0, 255},
    {20, 30, 24, 0, 0}
  },{
    {0, 0, 7, 45, 0},
    {8, 0, 11, 0, 255},
    {11, 10, 13, 0, 0},
    {13, 10, 20, 30, 255},
    {20, 45, 24, 0, 0}
  }
  // ... and so on
};

// Current light intensity table
// Number of values should match number of channels (i.e. {0,0,0} for 3 channels)
int lightValue[CHANNELS] = {0, 0};

// ----------------------- Functions ----------------------- 
void UpdateLights(long seconds)		// Update light intensity values
{
  for(int channel = 0; channel < CHANNELS; channel++) {  		// For each Channel
    for(int period = 0; period < MAXPERIODS; period++) {    // For each possible period
      long pEnd = GetSeconds(lightMatrix[channel][period][END_H], lightMatrix[channel][period][END_M], 0);	// Get period end time in seconds
      if(pEnd >= seconds) {				// Period is currently happening
        lightValue[channel] = lightMatrix[channel][period][INTENSITY];	// Set light to defined value
        break;		// Found correct period, don't check the rest
      } else {		// Period has not yet happened
        long pNextStart = GetSeconds(lightMatrix[channel][period+1][START_H], lightMatrix[channel][period+1][START_M], 0); // Get next period start time in seconds
        if(pNextStart > seconds) {					// Currently in between periods
          int preVal = lightMatrix[channel][period][INTENSITY];		// Old light value
          int postVal = lightMatrix[channel][period+1][INTENSITY];	// New light value
          int tDur = pNextStart - pEnd; 			// Transition duration
          int intensityDiff = postVal - preVal;	// Difference in light intesity between periods
          if(intensityDiff > 0) {		// Intensity increasing
            lightValue[channel] = (int)((seconds - pEnd) * ((float)intensityDiff / tDur));		// Light value
          } else {					// Intensity decreasing
            lightValue[channel] = preVal - (int)((seconds - pEnd) * ((float)(0-intensityDiff) / tDur));	// Light value
          }
          break;	// Found correct period, don't check the rest
        }
      }
    }
  }
}

long GetSeconds(int hours, int minutes, int seconds)	// Convert HH:mm:ss -> Seconds since midnight
{
  return ((long)hours * 60 * 60) + (minutes * 60) + seconds ;
}

// ----------------------- Setup ----------------------- 
void setup()
{
  // Set analog pins
  pinMode(L1Pin, OUTPUT);
  pinMode(L2Pin, OUTPUT);
  // ... and so on

  // Clock
  Wire.begin();
  RTC.begin();
  //RTC.adjust(DateTime(__DATE__, __TIME__));  // Set RTC time, only use for 1 (ONE) run. Will reset time at each device reset!

  Serial.begin(11500);
}

// ----------------------- Loop ----------------------- 
void loop(){
  // Get Time
  DateTime now = RTC.now();
  Hours = now.hour();
  Minutes = now.minute();
  Seconds = now.second();
  
  // Update actual light intensity to match values defined in lightValue[]
  UpdateLights(GetSeconds(Hours, Minutes, Seconds));
  analogWrite(L1Pin, lightValue[0]);
  analogWrite(L2Pin, lightValue[1]);
  // ... and so on
}
