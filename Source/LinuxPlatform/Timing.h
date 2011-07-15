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
#ifndef TIMING_H
#define TIMING_H

#include <time.h>


namespace Emunisce
{

class Time
{
public:

    Time();

    static Time Now();

    void Zero();

    unsigned int GetTotalMilliseconds();
    void SetTotalMilliseconds(unsigned int totalMilliseconds);

    void AddMilliseconds(int milliseconds);

private:

    static const clockid_t m_clockId = CLOCK_MONOTONIC;

    static int StaticInitialize();
    static int _Invoke_StaticInitialize;
    static timespec m_startTime;

    timespec m_currentTime;
};

class TimeSpan
{
public:

    static int GetElapsedMilliseconds(Time past);   ///<Gets milliseconds elapsed between past and Time::Now().
    static int GetElapsedMilliseconds(Time a, Time b);   ///<Gets milliseconds elapsed between a and b.  Can return a negative number if b is earlier than a.
};

}   //namespace Emunisce

#endif