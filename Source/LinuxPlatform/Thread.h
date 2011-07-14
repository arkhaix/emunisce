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
#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>


namespace Emunisce
{

class Thread
{
public:

	Thread();
	~Thread();

	void Start(void* param);
	void Stop();	///<Non-blocking
	void Join(unsigned int timeoutMilliseconds);	///<Blocking

	bool IsRunning();

	bool IsCallingThread(); ///<Returns true if called from the same thread that's running EntryPoint.


protected:

	virtual void EntryPoint(void* param) = 0;
	virtual void StopRequested() = 0;


	struct ThreadStartData
	{
		Thread* instance;
		void* userData;

		ThreadStartData()
		{
			instance = 0;
			userData = 0;
		}
	};

	ThreadStartData m_threadStartData;
	static void* StaticEntryPoint(void* param);

	static void StaticCleanup(void* param);

	pthread_t m_thread;
	bool m_isRunning;
};

}	//namespace Emunisce

#endif
