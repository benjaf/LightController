This is an 'advanced' 24 hour light scheduler designed for DIY Aquarium lighting

It allows for easy creation of pretty much any 24 hour light schedule imaginable within the limitations of the Arduino used.
This was done to provide easy support for  moonlight, sunrise/sunset, siesta etc.


Limitations:
- Schedule cannot exceed 24 hours (no handling of differences based on weekday, time of year etc.)
- Max number of channels: Defined by the number of PWM pins available on the Arduino used
- Max number of on/off cycles during a day: Any
- Precision: 
  - PWM duty cycle: 0-255
  - Time-to-PWM precision: 1 second (making the shortest fade timespan in which full PWM resolution is used 255 seconds)

This is not intended to be a complete aquarium or light controller,
it merely provides you with the code to handle light scheduling! 

You will need to download and install "RTClib" (https://github.com/adafruit/RTClib) for this to work.

I have NOT included any code for interfaces (Serial, LCD, etc.), nor any delays.
That would make the sketch more context-specific because these things depend on the users actual setup.
Communication via Serial is easy to find documentation on elsewhere.
It should be noted that this code contains no delays! If you start sending data (i.e. via Serial)
you might want to make sure this is only done at apropriate intervals.

This version uses 2 PWM pins because I only have 2 channels in my setup.
Fore more advanced changes, the user (you) should have basic understanding of programming.
This includes: 
- For-loops (and break conditions)
- Objects

As is probably evident from the code, I am not that experienced with writing software for microcontrollers.
I have written this code from an Object Oriented point of view to make it more aesily understandable.
Several optimizations are possible, but I tried to shoot for at least some degree of readability.
