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
#ifndef EVENT_H
#define EVENT_H

#include <pthread.h>


namespace Emunisce
{

class Event
{
public:

	Event(bool autoReset = false);
	~Event();

	void Set();		///<Sets state to signaled, allowing waiting threads to continue
	void Reset();	///<Sets state to non-signaled, causing threads to wait

	void Wait();

private:

    bool m_autoReset;

    bool m_signalled;
    pthread_mutex_t m_mutex;
    pthread_cond_t m_condition;
};

}   //namespace Emunisce

#endif
