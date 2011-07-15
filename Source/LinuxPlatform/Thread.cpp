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

#include "Timing.h"


Thread::Thread()
{
}

Thread::~Thread()
{
    if(IsRunning())
        Join(250);

    if(IsRunning())
    {
        pthread_cancel(m_thread);
    }
}


void Thread::Start(void* param)
{
    if(IsRunning())
        return;

    m_threadStartData.instance = this;
	m_threadStartData.userData = param;

    pthread_create(&m_thread, NULL, StaticEntryPoint, (void*)&m_threadStartData);
}

void Thread::Stop()
{
    if(IsRunning() == false)
        return;

    StopRequested();
}

void Thread::Join(unsigned int timeoutMilliseconds)
{
    Stop();
    pthread_join(m_thread, NULL);
    //todo: timeout support
}


bool Thread::IsRunning()
{
    return m_isRunning;
}


bool Thread::IsCallingThread()
{
    if( pthread_equal(pthread_self(), m_thread) )
        return true;

    return false;
}


void* Thread::StaticEntryPoint(void* param)
{
    ThreadStartData* data = (ThreadStartData*)param;
	if(data == NULL)
		return NULL;

	if(data->instance == NULL)
		return NULL;

    pthread_cleanup_push(StaticCleanup, (void*)data->instance);

    data->instance->m_isRunning = true;
	data->instance->EntryPoint( data->userData );

    pthread_cleanup_pop(1);
	pthread_exit(NULL);

	return NULL;
}

void Thread::StaticCleanup(void* param)
{
    Thread* instance = (Thread*)param;
    if(instance == NULL)
        return;

    instance->m_isRunning = false;
}


void Thread::Sleep(unsigned int milliseconds)
{
    Time startTime = Time::Now();

    long int elapsedMilliseconds = 0;

    do
    {
        pthread_yield();

        elapsedMilliseconds = Time::Now().GetTotalMilliseconds() - startTime.GetTotalMilliseconds();

    }   while(elapsedMilliseconds < milliseconds);
}

