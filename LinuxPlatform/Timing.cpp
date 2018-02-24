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

#include "PlatformTypes.h"

#include <math.h>   ///<fmod


// Time

timespec Time::m_startTime;
int Time::_Invoke_StaticInitialize = Time::StaticInitialize();

int Time::StaticInitialize()
{
    clock_gettime(m_clockId, &m_startTime);

    volatile int x = 7;
    return x;
}

Time Time::Now()
{
    Time result;
    clock_gettime(m_clockId, &result.m_currentTime);
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
    float result = (float)(m_currentTime.tv_sec - m_startTime.tv_sec) * 1000.f;
    result += (float)(m_currentTime.tv_nsec - m_startTime.tv_nsec) / 1e6;
    return result;
}

void Time::SetTotalMilliseconds(float totalMilliseconds)
{
    m_currentTime = m_startTime;

    int numSeconds = totalMilliseconds / 1000.f;
    m_currentTime.tv_sec += numSeconds;
    totalMilliseconds = fmodf(totalMilliseconds, 1000.f);

    int numNanoseconds = totalMilliseconds * 1e6;
    m_currentTime.tv_nsec += numNanoseconds;
    while(m_currentTime.tv_nsec >= 1e9)
    {
        m_currentTime.tv_sec++;
        m_currentTime.tv_nsec -= 1e9;
    }
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
