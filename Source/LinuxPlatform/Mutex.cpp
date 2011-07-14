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
#include "Mutex.h"
using namespace Emunisce;



Mutex::Mutex()
{
    pthread_mutexattr_init(&m_lockAttributes);
    pthread_mutexattr_settype(&m_lockAttributes, PTHREAD_MUTEX_RECURSIVE);

	pthread_mutex_init(&m_lock, &m_lockAttributes);
}

Mutex::~Mutex()
{
    pthread_mutex_destroy(&m_lock);
    pthread_mutexattr_destroy(&m_lockAttributes);
}


void Mutex::Acquire()
{
    pthread_mutex_lock(&m_lock);
}

void Mutex::Release()
{
    pthread_mutex_unlock(&m_lock);
}
