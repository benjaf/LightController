/*
 * Name:	LightController.ino
 * Author:	User "benjaf" at plantedtank.net forums
 * URL:		https://github.com/benjaf/LightController
 * This example is set up for 2 channels with a maximum of 10 points.
 * Anything that may require editing is labeled with ¤CHANGE¤
 
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE. 
 */

// ----------------------- RTC Library -----------------------
// Use Wire and RTClib (https://github.com/adafruit/RTClib):
// Please note that there are a significant differences between the original JeeLabs RTCLib and the Adafruit fork!
#include <Wire.h>
#include "RTClib.h"
#include "ChannelManager.h"

// ----------------------- Constants -----------------------
// ¤CHANGE¤
const int MaxChannels = 2;   // Max number of channels, change if more or less are required
const int MaxPoints = 10;    // Max number of light intensity points, change if more or less are required

// ----------------------- Variables -----------------------
// RTC
RTC_DS1307 RTC;

// Time
DateTime CurrentTime;

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
//
// If only 1 point is added, channel always maintains value of that point
//
// There is the option of which fade mode to use.
// Basically there are 2 modes: Linear and Exponential
// - Linear is what you would expect - 50% on = 50% duty cycle.
// - Exponential is an estimation of how our eyes react to brightness. The 'real' formula is actually inverse logarithmic,
// 	 but for our use exponential is close enough (feel free to add more fade modes if you disagree) - and much faster to compute!

Channel Channels[MaxChannels];
Point Points[MaxChannels][MaxPoints];

// Add more timing definitions here if more channels are added:
// ¤CHANGE¤
void InitializeChannels(int channels) {
	// Channel 0: (Example)
	// This is an example of a typical lighting schedule (Lights on 08:30 - 19:00 w. 30 minutes of sunrise / sunset added)
	// using linear fading on pin 10.
	
	//int channelNo = 0;
	//int pin = 10;
	//Channels[channelNo] = Channel(pin, MaxPoints, fademode_linear, Points[channelNo]);
	//Channels[channelNo].AddPoint(8, 0, 0);
	//Channels[channelNo].AddPoint(8, 30, 255);
	//Channels[channelNo].AddPoint(19, 0, 255);
	//Channels[channelNo].AddPoint(19, 30, 0);
  	
	// Channel 0:
	int channelNo = 0;	// Currently editing channel 0
	int pin = 10;		// Channel 0 uses pin 10
	Channels[channelNo] = Channel(pin, MaxPoints, fademode_linear, Points[channelNo]);	// Initialize channel and choose FadeMode
	Channels[channelNo].AddPoint(8, 0, 0);	// Add Point (can also use decimal values ranging from 0 to 1 if you prefer)
	Channels[channelNo].AddPoint(9, 0, 255);
	Channels[channelNo].AddPoint(19, 30, 255);
	Channels[channelNo].AddPoint(20, 0, 0);
	
	// Channel 1:
	channelNo = 1;	// Currently editing channel 1
	pin = 11;		// Channel 1 uses pin 11
	Channels[channelNo] = Channel(pin, MaxPoints, fademode_linear, Points[channelNo]);
	Channels[channelNo].AddPoint(8, 0, 0);
	Channels[channelNo].AddPoint(8, 30, 150);
	Channels[channelNo].AddPoint(11, 0, 255);
	Channels[channelNo].AddPoint(11, 15, 0);
	Channels[channelNo].AddPoint(12, 30, 0);
	Channels[channelNo].AddPoint(12, 45, 255);
	Channels[channelNo].AddPoint(19, 30, 150);
	Channels[channelNo].AddPoint(20, 0, 0);
}

// ----------------------- Functions -----------------------
long lastUpdateTime = 0;

// Update light intensity values
void UpdateLights(DateTime currentTime)
{
	long now = Seconds(currentTime.hour(), currentTime.minute(), currentTime.second());	// Convert current time to seconds since midnight
	if(now != lastUpdateTime)  	// Perform update only if there is a perceivable change in time (no point wasting clock cycles otherwise)
	{
		for(int channel = 0; channel < MaxChannels; channel++)    		// For each Channel
		{
			analogWrite(Channels[channel].GetPin(), Channels[channel].GetLightIntensityInt(now));	// Get updated light intensity and write value to pin (update is performed when reading value)
		}
	}
	lastUpdateTime = now;
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
}

// ----------------------- Loop -----------------------
void loop() {
	// Get current time
	CurrentTime = RTC.now();
	
	// Update lights
	UpdateLights(CurrentTime);
}
