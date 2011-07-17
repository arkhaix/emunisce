/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Timing.h"
using namespace Emunisce;


// Time

LARGE_INTEGER Time::m_startTime;
double Time::m_millisecondsPerCount;
double Time::m_countsPerMillisecond;
int Time::_Invoke_StaticInitialize = Time::StaticInitialize();

int Time::StaticInitialize()
{
	LARGE_INTEGER countsPerSecond;
	QueryPerformanceFrequency(&countsPerSecond);

	m_countsPerMillisecond = (double)countsPerSecond.QuadPart / 1000.0;
	m_millisecondsPerCount = 1.0 / m_countsPerMillisecond;

	QueryPerformanceCounter(&m_startTime);

	volatile int x = 7;
	return x;
}

Time Time::Now()
{
	Time result;
	QueryPerformanceCounter(&result.m_currentTime);
	return result;
}

Time::Time()
{
	m_currentTime = m_startTime;
}

void Time::Zero()
{
	m_currentTime = m_startTime;
}

float Time::GetTotalMilliseconds()
{
	LARGE_INTEGER elapsedTime;
	elapsedTime.QuadPart = m_currentTime.QuadPart - m_startTime.QuadPart;

	double result = (double)elapsedTime.QuadPart * m_millisecondsPerCount;

	return (float)result;
}

void Time::SetTotalMilliseconds(float totalMilliseconds)
{
	m_currentTime = m_startTime;

	m_currentTime.QuadPart += (LONGLONG)((double)totalMilliseconds * m_countsPerMillisecond);
}

void Time::AddMilliseconds(float milliseconds)
{
	SetTotalMilliseconds( GetTotalMilliseconds() + milliseconds );
}


// TimeSpan

float TimeSpan::GetElapsedMilliseconds(Time past)
{
	return GetElapsedMilliseconds(past, Time::Now());
}

float TimeSpan::GetElapsedMilliseconds(Time a, Time b)
{
	return b.GetTotalMilliseconds() - a.GetTotalMilliseconds();
}
