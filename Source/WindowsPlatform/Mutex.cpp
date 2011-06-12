#include "Mutex.h"

#include "windows.h"


class Mutex_Private
{
public:

	LPCRITICAL_SECTION CriticalSection;
};


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
