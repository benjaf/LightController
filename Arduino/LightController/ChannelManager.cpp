#include "Arduino.h"
#include "ChannelManager.h"

// ---------------- Point ------------------------
Point::Point()
{
	_minutes = 0;
	_intensity = 0;
}

// Initialize Point with given values
Point::Point(byte h, byte m, float intensity)
{
	_minutes = (60 * (int)h) + m;
	if(intensity > 0) 
	{
		_intensity = intensity / (float)255;
	}
	else 
	{
		_intensity = intensity;
	}
}

long Point::GetTimeSeconds()
{
	return _minutes * 60;
}

byte Point::GetHours()
{
	return (byte)(_minutes / 60);
}

byte Point::GetMinutes()
{
	return (byte)(_minutes % 60);
}

float Point::GetIntensity()
{
	return _intensity;
}

byte Point::GetIntensityInt()
{
	return (byte)((float) 255 * _intensity);
}

byte Point::GetIntensityPercent()
{
	return (byte)(100 * _intensity);
}

bool Point::IsValid()
{
	return _minutes >= 0 && _minutes < 60 * 24 && _intensity >= 0 && _intensity <= 1;
}

bool Point::IsZero()
{
	return _minutes == 0 && _intensity == 0;
}

void Point::PrintPoint()
{
	Serial.print("Point: ");
	Serial.print(GetHours());
	Serial.print("-");
	Serial.print(GetMinutes());
	Serial.print("-");
	Serial.print(GetIntensityPercent());
	if(IsValid())
	{
		Serial.println(" VALID");
	}
	else
	{
		Serial.println(" NOT VALID");
	}
}

// ------------------- Channel ---------------------------

Channel::Channel() {}

// Initialize channel w. no points
Channel::Channel(int pin, int maxLength, FadeMode fadeMode, Point* m)
{
	_pin = pin;
	_lightValue = 0;
	_fadeMode = fadeMode;
	_maxLength = maxLength;
	_length = 0;
	_currentPosition = 1;
	_storage = m;
}

int Channel::GetPin()
{
	return _pin;
}

// Add Point to first available position (wrapper function)
void Channel::AddPoint(int h, int m, float intensity)
{
	Point p = Point(h, m, intensity);
	if(_length == 0)
	{
		_previous = p;
		_next = p;
		_currentPosition = 1;
		_length = 1;
	}
	else
	{
		_previous = p;
		_currentPosition++;
		_length++;
	}
	SetPoint(_currentPosition, p);
}

void Channel::SetPoint(int index, int h, int m, float intensity)
{
	SetPoint(index, Point(h, m, intensity));
}

void Channel::SetPoint(int index, Point p)
{
	_storage[index] = p;
}

void Channel::ClearPoint(int index)
{
	SetPoint(index, Point());
}

Point Channel::GetPoint(int index)
{
	return _storage[index];
}

void Channel::GoToCurrentPosition(long time)
{
	if(_length <= 1) return;
	while(true)
	{
		if (_previous.GetTimeSeconds() <= time && _next.GetTimeSeconds() > time)
		{
			// Between 'previous' point and 'next'
			break;
		}
		else if (_previous.GetTimeSeconds() <= time && _next.GetTimeSeconds() < _previous.GetTimeSeconds())
		{
			// Between 'previous' point and 'next', currently before midnight and 'next' beeing after midnight
			break;
		}
		else if (_previous.GetTimeSeconds() > time && _next.GetTimeSeconds() > time && _previous.GetTimeSeconds() > _next.GetTimeSeconds())
		{
			// Between 'previous' point and 'next', currently after midnight and 'previous' point being before midnight
			break;
		}
		else
		{
			// Better luck next time
			MoveForward();
		}
	}
}

void Channel::MoveForward()
{
	int nextPosition = 1;
	if(_currentPosition < _length - 1)
	{
		_currentPosition++;
		nextPosition = _currentPosition + 1;
	}
	else if(_currentPosition == _length - 1)
	{
		_currentPosition++;
		nextPosition = 1;
	}
	else if(_currentPosition == _length)
	{
		_currentPosition = 1;
		nextPosition = _currentPosition + 1;
	}
	_previous = _storage[_currentPosition];
	_next =  _storage[nextPosition];
}

int Channel::GetLightIntensityInt(long time)
{
	UpdateCurrentLightValue(time);
	return _lightValue;
}

float Channel::CorrectForFadeMode(float intensity)
{
	switch(_fadeMode)
	{
		case fademode_exponential:
			return intensity * intensity;
		default:
			return intensity;
	}
}

void Channel::UpdateData()
{
	_length = 1;
	Point _c;
	Point _n;
	for(_length; _length < _maxLength; _length++)
	{
		// End of channel has been reached if:
		_c = _storage[_length];
		//_c.PrintPoint();
		if(!_c.IsValid() || _length == _maxLength)
		{
			// Current point is invalid (can only happen at the first one) or Max length of channel has been reached
			return;
		}
		_n = _storage[_length + 1];
		if(!_n.IsValid() || _n.GetTimeSeconds() < _c.GetTimeSeconds() || (_c.IsZero() && _n.IsZero()))
		{
			// Next point is invalid or earlier than current point (different kind of invalid)
			return;
		}
	}
}

int Channel::GetLength()
{
	return _length;
}

void Channel::UpdateCurrentLightValue(long time)
{
	GoToCurrentPosition(time);
	long timeDiff = 0;
	long prevPointTime = _previous.GetTimeSeconds();
	float prevPointIntensity = _previous.GetIntensity();
	long nextPointTime = _next.GetTimeSeconds();
	float nextPointIntensity = _next.GetIntensity();

	if(prevPointTime > nextPointTime)   // Midnight rollover
	{
		timeDiff = (nextPointTime + 24 * 60 * 60) - prevPointTime;
	}
	else
	{
		timeDiff = nextPointTime - prevPointTime;
	}

	long progress = 0;
	if(time >= prevPointTime)   // Currently before midnight
	{
		progress = time - prevPointTime;
	}
	else     // Currently after midnight
	{
		progress = (time + 24 * 60 * 60) - prevPointTime;
	}
	float intensityDiff = nextPointIntensity - prevPointIntensity;	// Difference in light intesity between points
	float intensity = prevPointIntensity + (progress * (intensityDiff / timeDiff)); // Current intensity
	_lightValue = 255 * CorrectForFadeMode(intensity);		// Light value
}

void Channel::Reset()
{
	_currentPosition = 1;
	_previous = GetPoint(1);
	_next = GetPoint(1);
}