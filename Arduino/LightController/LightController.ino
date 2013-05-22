/*
 * Name:	LightController.ino
 * Author:	User "benjaf" at plantedtank.net forums
 * URL:		https://github.com/benjaf/LightController
 * This example is set up for 2 channels with 5 lighting periods.
 * Anything that may require editing is labeled with ¤CHANGE¤
 */


// ----------------------- RTC Library -----------------------
// One could use RTCdue (https://github.com/oskarirauta/RTCdue) to enable Arduino Due support
// I had to add following lines to RTCdue.h to make it compile without errors:
// #define PIN_WIRE_SDA 4
// #define PIN_WIRE_SCL 5
//#include "RTCdue.h"
// Alternatively, you can use Wire and RTClib (https://github.com/adafruit/RTClib) which work well with anything else:
#include <Wire.h>
#include "RTClib.h"

// ----------------------- Constants -----------------------
// ¤CHANGE¤
const int MaxChannels = 2;   // Max number of channels, change if more or less are required
const int MaxPoints = 10;    // Max number of light intensity points, change if more or less are required

// ----------------------- Variables -----------------------
// RTC
RTC_DS1307 RTC;

// Time
DateTime CurrentTime;

// ----------------------- Classes -----------------------
// Point data type
class Point {
	public:
		int Hour;
		int Minute;
		int Intensity;
		
		// Initialize Point with 24:00 OFF
		Point() {
			Hour = 24;
			Minute = 0;
			Intensity = 0;
		}
		
		// Initialize Point with given values
		Point(int h, int m, int intensity) {
			Hour = h;
			Minute = m;
			Intensity = intensity;
		}
		
		// Get Point time in seconds since midnight
		long GetSeconds() {
			return Seconds(Hour, Minute, 0);
		}
};

// Channel data type
class Channel {
	public:
		int Pin;
		int LightValue;
		Point Schedule[MaxPoints];
   
   		// Initialize channel full of 24:00 OFF Points
		Channel() {
			Pin = 0;
			LightValue = 0;
			for(int point = 0; point < MaxPoints; point++) {
				Schedule[point] = Point();
			}
		}
		
		// Add Point at given position
		void SetPoint(int row, Point p) {
			Schedule[row] = p;
		}
		
		// Add Point to first available position (wrapper function)
		void AddPoint(int h, int m, int intensity) {
			AddPoint(Point(h, m, intensity));
		}
		
		// Add Point to first available position
		void AddPoint(Point p) {
			for(int i = 0; i < MaxPoints; i++) {
				if(Schedule[i].Hour == 24) {
					Schedule[i] = p;
					break;
				}
			}
		}
};

// ----------------------- Lights -----------------------

// Schedule Point format: (Hour, Minute, Intensity)
// Difference in intensity between points is faded/increased gradually
// Example: 
// 	Channels[0].AddPoint(8, 0, 0);
//	Channels[0].AddPoint(8, 30, 255);
//	Channels[0].AddPoint(11, 0, 255);
//  ...
//
// Explanation:
//  00:00 - 08:00 -> Light OFF
//  08:00 - 08:30 -> Increment light towards Fully ON
//  08:30 - 11:00 -> Light Fully ON
//
// Min intensity value: 0 (OFF)
// Max intensity value: 255 (Fully ON)
// If no Point is made at 00:00, channel is assumed to be OFF until first Point is encountered
// If last Point is not 24:00, channel is assumed to be OFF after last Point is encountered
//
// Any Point with 24:00 will be considered last.
// Point order must always be chronological overlaps.
// Any row not explicitly set will contain 24:00 OFF, which is OK as long as anyting up to that point is valid.
// On creation, all schedules are valid and OFF (all points are 24:00 OFF).
//
// Example: 
// 	Channels[0].AddPoint(0, 0, 255);
//	Channels[0].AddPoint(24, 0, 255);
// Explanation:
// Channel is ALWAYS FULLY ON.
// As the second Point never ends, the remaining rows are only there to fill out the necessary space and their content does not matter.


Channel Channels[MaxChannels];

// Add more timing definitions here if more channels are added:
// ¤CHANGE¤

void InitializeChannels(int channels) {
	// Channel 0: (Example)
	// This is an example of a typical lighting schedule (Lights on 08:30 - 19:00 w. 30 minutes of sunrise / sunset added)
	//Channels[0].Pin = 11;
	//Channels[0].AddPoint(8, 0, 0);
	//Channels[0].AddPoint(8, 30, 255);
  	//Channels[0].AddPoint(19, 0, 255);
  	//Channels[0].AddPoint(19, 30, 0);
  	
	// Channel 0:
	Channels[0].Pin = 9;
	Channels[0].AddPoint(8, 0, 0);
	Channels[0].AddPoint(8, 30, 255);
	Channels[0].AddPoint(11, 0, 0);
	Channels[0].AddPoint(11, 10, 0);
	Channels[0].AddPoint(13, 0, 0);
	Channels[0].AddPoint(13, 10, 255);
	Channels[0].AddPoint(20, 0, 255);
	Channels[0].AddPoint(20, 30, 0);
  
	// Channel 1:
	Channels[1].Pin = 10;
	Channels[1].AddPoint(7, 45, 0);
	Channels[1].AddPoint(8, 0, 255);
	Channels[1].AddPoint(11, 0, 0);
	Channels[1].AddPoint(11, 10, 0);
	Channels[1].AddPoint(13, 0, 0);
	Channels[1].AddPoint(13, 10, 255);
	Channels[1].AddPoint(20, 30, 255);
	Channels[1].AddPoint(20, 45, 0);
}

// ----------------------- Functions -----------------------
// Update light intensity values 
void UpdateLights(DateTime currentTime)	{
	long now = Seconds(currentTime.hour(), currentTime.minute(), currentTime.second());	// Convert current time to seconds since midnight
        for(int channel = 0; channel < MaxChannels; channel++) {  		// For each Channel
		Channel* c = &Channels[channel];  // Store pointer to current channel here for easy access. Must use "c->" rather than "c." to access properties
		for(int point = 0; point < MaxPoints; point++) {    // For each possible point
			long pTime = c->Schedule[point].GetSeconds();	// Get time of this point
			if(pTime > now) {				// First entry has not yet happened - this means no 00:00 was defined and cannel is assumed to be OFF
				c->LightValue = 0;	// Set light to 0
				break;	// Don't check the rest of the points
			} else if(point + 1 >= MaxPoints) {	// We reached the end of the array, which means no 24:00 was defined. Assume channel is OFF
				c->LightValue = 0;	// Set light to 0
				break;	// There are no more points to check anyway
			} else {
				long pNextTime = c->Schedule[point+1].GetSeconds();	// Get time of the next point
				if(pNextTime >= now) {	// We are currently between this point and the next
					int preVal = c->Schedule[point].Intensity;		// Old light value
					int postVal = c->Schedule[point+1].Intensity;	// New light value
					int tDur = pNextTime - pTime; 			// Transition duration
					int intensityDiff = postVal - preVal;	// Difference in light intesity between points
					if(intensityDiff > 0) {		// Intensity increasing
						c->LightValue = (int)((now - pTime) * ((float)intensityDiff / tDur));		// Light value
					} else {					// Intensity decreasing
						c->LightValue = preVal - (int)((now - pTime) * ((float)(0-intensityDiff) / tDur));	// Light value
					}
					break;	// Found correct points, don't check the rest
				}
				// If we get this far, the correct points were not yet found. Retry with next point.
			}
		}
		analogWrite(c->Pin, c->LightValue);	// Write value to pin
	}
}

// Convert HH:mm:ss -> Seconds since midnight
long Seconds(int hours, int minutes, int seconds) {
	return ((long)hours * 60 * 60) + (minutes * 60) + seconds ;
}

// ----------------------- Setup -----------------------
void setup() {
	// Initialize channel schedules
	InitializeChannels(MaxChannels);
	
	// Clock
	Wire.begin();
	RTC.begin();
	//RTC.adjust(DateTime(__DATE__, __TIME__));  // Set RTC time to sketch compilation time, only use for 1 (ONE) run. Will reset time at each device reset!

	// Debug monitor
	//Serial.begin(9600);
}

// ----------------------- Loop -----------------------
void loop() {
	// Get current time
	CurrentTime = RTC.now();
        
	// Update lights
	UpdateLights(CurrentTime);
	
        // Debug
        /*
        delay(1000);
        Serial.print(CurrentTime.hour());
        Serial.print(":");
        Serial.print(CurrentTime.minute());
        Serial.print(":");
        Serial.println(CurrentTime.second());
        for(int i = 0; i < MaxChannels; i++) {
          Serial.print("Channel ");
          Serial.print(i);
          Serial.print(": ");
          Serial.println(Channels[i].LightValue);
        }
        */
}
