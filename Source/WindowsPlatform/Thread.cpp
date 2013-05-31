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
#include "Thread.h"
using namespace Emunisce;


Thread::Thread()
{
	m_threadHandle = nullptr;
	m_threadId = 0;
}

Thread::~Thread()
{
	if(m_threadHandle != nullptr)
	{
		if(IsRunning())
			Join(250);

		if(IsRunning())
		{
			TerminateThread(m_threadHandle, 3);
			CloseHandle(m_threadHandle);
			m_threadHandle = nullptr;
		}
	}
}


void Thread::Start(void* param)
{
	DWORD exitCode = STILL_ACTIVE;
	if(m_threadHandle != nullptr && GetExitCodeThread(m_threadHandle, &exitCode) && exitCode == STILL_ACTIVE)
		return;

	m_threadStartData.instance = this;
	m_threadStartData.userData = param;

	m_threadHandle = CreateThread(nullptr, 0, StaticEntryPoint, (LPVOID)&m_threadStartData, 0, &m_threadId);
}

void Thread::Stop()
{
	if(m_threadHandle == nullptr)
		return;

	DWORD exitCode = STILL_ACTIVE;
	if(GetExitCodeThread(m_threadHandle, &exitCode) && exitCode != STILL_ACTIVE)
		return;

	StopRequested();
}

void Thread::Join(unsigned int timeoutMilliseconds)
{
	if(IsRunning() == false)
		return;

	Stop();

	WaitForSingleObject(m_threadHandle, timeoutMilliseconds);
}


bool Thread::IsRunning()
{
	if(m_threadHandle == nullptr)
		return false;

	DWORD exitCode = STILL_ACTIVE;
	if(GetExitCodeThread(m_threadHandle, &exitCode) && exitCode == STILL_ACTIVE)
		return true;

	return false;
}


bool Thread::IsCallingThread()
{
    if(GetCurrentThreadId() == m_threadId)
        return true;

    return false;
}


void Thread::Sleep(unsigned int milliseconds)
{
	::Sleep((DWORD)milliseconds);
}


DWORD WINAPI Thread::StaticEntryPoint(LPVOID param)
{
	ThreadStartData* data = (ThreadStartData*)param;
	if(data == nullptr)
		return 1;

	if(data->instance == nullptr)
		return 2;

	class HandleCloser
	{
	public:

		HANDLE& _Handle;
		HandleCloser(HANDLE& handle) : _Handle(handle) { }
		~HandleCloser() { CloseHandle(_Handle); _Handle = nullptr; }
	};

	HandleCloser handleCloser(data->instance->m_threadHandle);

	data->instance->EntryPoint( data->userData );

	return 0;
}
