#include "sound.h"

#include "windows.h"	///<For critical sections.  The audio buffer needs to be locked.

#include "../common/machine.h"
#include "../memory/memory.h"

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
	m_ticksSinceLastSample += ticks;
	m_ticksUntilNextSample -= ticks;

	if(m_ticksUntilNextSample <= 0)
	{
		m_ticksUntilNextSample += m_ticksPerSample;
		
		float secondsSinceLastSample = (float)m_ticksSinceLastSample / (float)m_machine->GetTicksPerSecond();
		m_ticksSinceLastSample = 0;

		//Update the generators

		//Get the samples

		//Mix the samples

		//Output the final sample

		//Update the index
		m_nextSampleIndex++;
		if(m_nextSampleIndex >= m_activeAudioBuffer->BufferSize)
		{
			//Reached the end of the buffer.  Swap them.
			EnterCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
				AudioBuffer* temp = m_stableAudioBuffer;
				m_stableAudioBuffer = m_activeAudioBuffer;
				m_activeAudioBuffer = temp;
			LeaveCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);

			m_nextSampleIndex = 0;
		}
	}
}

void Sound::SetMachine(Machine* machine)
{
	m_machine = machine;
	m_memory = machine->GetMemory();

	m_ticksPerSample = (float)machine->GetTicksPerSecond() / 44100.f;
	m_ticksUntilNextSample = m_ticksPerSample;
	m_ticksSinceLastSample = 0;


	m_memory->SetRegisterLocation(0x10, &m_sound1Sweep, true);
	m_memory->SetRegisterLocation(0x11, &m_sound1Length, true);
	m_memory->SetRegisterLocation(0x12, &m_sound1Envelope, true);
	m_memory->SetRegisterLocation(0x13, &m_sound1FrequencyLow, true);
	m_memory->SetRegisterLocation(0x14, &m_sound1FrequencyHigh, true);

	m_memory->SetRegisterLocation(0x16, &m_sound2Length, true);
	m_memory->SetRegisterLocation(0x17, &m_sound2Envelope, true);
	m_memory->SetRegisterLocation(0x18, &m_sound2FrequencyLow, true);
	m_memory->SetRegisterLocation(0x19, &m_sound2FrequencyHigh, true);

	m_memory->SetRegisterLocation(0x1a, &m_sound3Enable, true);
	m_memory->SetRegisterLocation(0x1b, &m_sound3Length, true);
	m_memory->SetRegisterLocation(0x1c, &m_sound3Level, true);
	m_memory->SetRegisterLocation(0x1d, &m_sound3FrequencyLow, true);
	m_memory->SetRegisterLocation(0x1e, &m_sound3FrequencyHigh, true);

	m_memory->SetRegisterLocation(0x20, &m_sound4Length, true);
	m_memory->SetRegisterLocation(0x21, &m_sound4Envelope, true);
	m_memory->SetRegisterLocation(0x22, &m_sound4Nfc, true);
	m_memory->SetRegisterLocation(0x23, &m_sound4Initialize, true);

	m_memory->SetRegisterLocation(0x24, &m_soundOutputLevels, true);
	m_memory->SetRegisterLocation(0x25, &m_soundOutputTerminals, true);
	m_memory->SetRegisterLocation(0x26, &m_soundEnable, true);
}
