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
#include "Event.h"
using namespace Emunisce;


Event::Event(bool autoReset)
{
    m_autoReset = autoReset;

    m_signalled = false;

    pthread_mutex_init(&m_mutex, nullptr);
    pthread_cond_init(&m_condition, nullptr);
}

Event::~Event()
{
    pthread_cond_destroy(&m_condition);
    pthread_mutex_destroy(&m_mutex);
}


void Event::Set()
{
    pthread_mutex_lock(&m_mutex);

    m_signalled = true;
    pthread_cond_signal(&m_condition);

    pthread_mutex_unlock(&m_mutex);
}

void Event::Reset()
{
    pthread_mutex_lock(&m_mutex);

    m_signalled = false;

    pthread_mutex_unlock(&m_mutex);
}


void Event::Wait()
{
    pthread_mutex_lock(&m_mutex);

    while(m_signalled == false)
    {
        pthread_cond_wait(&m_condition, &m_mutex);
    }

    if(m_autoReset == true)
    {
        m_signalled = false;
    }

    pthread_mutex_unlock(&m_mutex);
}

