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

unsigned int Time::GetTotalMilliseconds()
{
    unsigned int result = (unsigned int)(m_startTime.tv_sec - m_currentTime.tv_sec) * 1000;
    result += (m_currentTime.tv_nsec - m_startTime.tv_nsec) / 1e6;
    return result;
}

void Time::SetTotalMilliseconds(unsigned int totalMilliseconds)
{
    m_currentTime = m_startTime;

    m_currentTime.tv_sec += (totalMilliseconds / 1000);
    totalMilliseconds /= 1000;

    m_currentTime.tv_nsec += (totalMilliseconds * 1e6);
    while(m_currentTime.tv_nsec > 1e9)
    {
        m_currentTime.tv_sec++;
        m_currentTime.tv_nsec -= 1e9;
    }
}

void Time::AddMilliseconds(int milliseconds)
{
    SetTotalMilliseconds( GetTotalMilliseconds() + milliseconds );
}


// TimeSpan

int TimeSpan::GetElapsedMilliseconds(Time past)
{
    return GetElapsedMilliseconds(past, Time::Now());
}

int TimeSpan::GetElapsedMilliseconds(Time a, Time b)
{
    return b.GetTotalMilliseconds() - a.GetTotalMilliseconds();
}
