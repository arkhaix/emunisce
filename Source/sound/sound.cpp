#include "sound.h"

#include "windows.h"	///<For critical sections.  The audio buffer needs to be locked.

Sound::Sound()
{
	m_activeAudioBuffer = &m_audioBuffer[0];
	m_stableAudioBuffer = &m_audioBuffer[1];

	m_audioBufferLock = (void*)new CRITICAL_SECTION();
	InitializeCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
}

Sound::~Sound()
{
	DeleteCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
	delete (LPCRITICAL_SECTION)m_audioBufferLock;
}

void Sound::Initialize()
{
}

void Sound::Run(int ticks)
{
}

void Sound::SetMachine(Machine* machine)
{
	m_machine = machine;
}
