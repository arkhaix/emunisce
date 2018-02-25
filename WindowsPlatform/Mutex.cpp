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

#include "windows.h"



namespace Emunisce
{

class Mutex_Private
{
public:

	LPCRITICAL_SECTION CriticalSection;
};

}	//namespace Emunisce


Mutex::Mutex()
{
	m_private = new Mutex_Private();

	m_private->CriticalSection = new CRITICAL_SECTION();
	InitializeCriticalSection(m_private->CriticalSection);
}

Mutex::~Mutex()
{
	DeleteCriticalSection(m_private->CriticalSection);
	delete m_private->CriticalSection;

	delete m_private;
}


void Mutex::Acquire()
{
	EnterCriticalSection(m_private->CriticalSection);
}

void Mutex::Release()
{
	LeaveCriticalSection(m_private->CriticalSection);
}