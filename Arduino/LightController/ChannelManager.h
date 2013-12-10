
#include "Arduino.h"

// ----------------------- Fade Mode -----------------------
enum FadeMode
{
    fademode_linear,
    fademode_exponential
};

// ----------------------- Point -----------------------
class Point
{
public:
	Point();
	Point(byte h, byte m, float intensity);
	long GetTimeSeconds();
	byte GetHours();
	byte GetMinutes();
	float GetIntensity();
	byte GetIntensityPercent();
	byte GetIntensityInt();
	bool IsValid();
	bool IsZero();
	void PrintPoint();

private:
	long _minutes;
	float _intensity;
};

// Channel data type
class Channel
{
public:
	Channel();
	Channel(int pin, int maxLength, FadeMode fadeMode, Point* m);
	int GetPin();
	void AddPoint(int h, int m, float intensity);
	void SetPoint(int index, int h, int m, float intensity);
	void SetPoint(int index, Point p);
	void ClearPoint(int index);
	Point GetPoint(int index);
	void GoToCurrentPosition(long time);
	void MoveForward();
	int GetLightIntensityInt(long time);
	float CorrectForFadeMode(float intensity);
	void UpdateCurrentLightValue(long time);
	void UpdateData();
	int GetLength();
	void Reset();

private:
	int _pin;
	int _lightValue;
	FadeMode _fadeMode;
	int _currentPosition;
	int _length;
	int _maxLength;
	Point _previous;
	Point _next;
	Point* _storage;
};
