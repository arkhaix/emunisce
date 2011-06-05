#include "sound.h"

#include "windows.h"	///<For critical sections.  The audio buffer needs to be locked.

#include "../common/machine.h"
#include "../memory/memory.h"

#include "sound1.h"
#include "sound2.h"
#include "sound3.h"
#include "sound4.h"

#include "channelController.h"

#if 0
#include <cstdio>
#define TRACE_REGISTER_WRITE printf(__FUNCTION__ "(%02X) nr52(%02X)\n", value, m_nr52);
#else
#define TRACE_REGISTER_WRITE
#endif


Sound::Sound()
{
	m_activeAudioBuffer = &m_audioBuffer[0];
	m_stableAudioBuffer = &m_audioBuffer[1];

	m_audioBufferLock = (void*)new CRITICAL_SECTION();
	InitializeCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);

	m_hasPower = true;

	for(int i=0;i<4;i++)
		m_channelDisabler[i] = new ChannelController(m_nr52, i);

	m_sound1 = new Sound1();
	m_sound2 = new Sound2();
	m_sound3 = new Sound3();
	m_sound4 = new Sound4();
}

Sound::~Sound()
{
	delete m_sound1;
	delete m_sound2;
	delete m_sound3;
	delete m_sound4;

	for(int i=0;i<4;i++)
		delete m_channelDisabler[i];

	DeleteCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
	delete (LPCRITICAL_SECTION)m_audioBufferLock;
}

void Sound::Initialize()
{
	m_nextSampleIndex = 0;

	m_audioBufferCount = 0;

	m_frameSequencerPeriod = 8192;	///<8192 = 512Hz = (4194304 ticks per second / 512Hz).
	m_frameSequencerTimer = m_frameSequencerPeriod;
	m_frameSequencerPosition = 0;

	m_inaccessible = 0xff;

	SetNR50(0x00);
	SetNR51(0x00);
	SetNR52(0xf0);

	m_sound1->Initialize(m_channelDisabler[0]);
	m_sound2->Initialize(m_channelDisabler[1]);
	m_sound3->Initialize(m_channelDisabler[2]);
	m_sound4->Initialize(m_channelDisabler[3]);
}

void Sound::SetMachine(Machine* machine)
{
	m_machine = machine;
	m_memory = machine->GetMemory();

	m_ticksPerSample = (float)machine->GetTicksPerSecond() / (float)SamplesPerSecond;
	m_ticksUntilNextSample = m_ticksPerSample;
	m_ticksSinceLastSample = 0;

	m_memory->SetRegisterLocation(0x24, &m_nr50, false);
	m_memory->SetRegisterLocation(0x25, &m_nr51, false);
	m_memory->SetRegisterLocation(0x26, &m_nr52, false);

	m_memory->SetRegisterLocation(0x27, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x28, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x29, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2a, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2b, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2c, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2d, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2e, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2f, &m_inaccessible, false);

	m_sound1->SetMachine(machine);
	m_sound2->SetMachine(machine);
	m_sound3->SetMachine(machine);
	m_sound4->SetMachine(machine);
}

void Sound::Run(int ticks)
{
	//Update the frame sequencer

	m_frameSequencerTimer -= ticks;
	while(m_frameSequencerTimer <= 0)
	{
		m_frameSequencerTimer += m_frameSequencerPeriod;

		if(m_hasPower)
		{

			m_frameSequencerPosition++;
			if(m_frameSequencerPosition > 7)
				m_frameSequencerPosition = 0;

			if(m_frameSequencerPosition == 0 || m_frameSequencerPosition == 2 || 
				m_frameSequencerPosition == 4 || m_frameSequencerPosition == 6)
			{
				m_sound1->TickLength();
				m_sound2->TickLength();
				m_sound3->TickLength();
				m_sound4->TickLength();
			}

			if(m_frameSequencerPosition == 2 || m_frameSequencerPosition == 6)
			{
				m_sound1->TickSweep();
			}

			if(m_frameSequencerPosition == 7)
			{
				m_sound1->TickEnvelope();
				m_sound2->TickEnvelope();
				//3 has no envelope
				m_sound4->TickEnvelope();
			}

		}	//if(m_hasPower)
	}


	//Run the components

	m_sound1->Run(ticks);
	m_sound2->Run(ticks);
	m_sound3->Run(ticks);
	m_sound4->Run(ticks);
}

AudioBuffer Sound::GetStableAudioBuffer()
{
	EnterCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
		AudioBuffer result = *m_stableAudioBuffer;
	LeaveCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);

	return result;
}

int Sound::GetAudioBufferCount()
{
	return m_audioBufferCount;
}


void Sound::SetNR10(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR10(value);
}

void Sound::SetNR11(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR11(value);
}

void Sound::SetNR12(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR12(value);
}

void Sound::SetNR13(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR13(value);
}

void Sound::SetNR14(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR14(value);
}


void Sound::SetNR21(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound2->SetNR21(value);
}

void Sound::SetNR22(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound2->SetNR22(value);
}

void Sound::SetNR23(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound2->SetNR23(value);
}

void Sound::SetNR24(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound2->SetNR24(value);
}


void Sound::SetNR30(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR30(value);
}

void Sound::SetNR31(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR31(value);
}

void Sound::SetNR32(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR32(value);
}

void Sound::SetNR33(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR33(value);
}

void Sound::SetNR34(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR34(value);
}


void Sound::SetNR41(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound4->SetNR41(value);
}

void Sound::SetNR42(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound4->SetNR42(value);
}

void Sound::SetNR43(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound4->SetNR43(value);
}

void Sound::SetNR44(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound4->SetNR44(value);
}


void Sound::SetNR50(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_hasPower == false)
		return;

	m_nr50 = value;
}

void Sound::SetNR51(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_hasPower == false)
		return;

	m_nr51 = value;
}

void Sound::SetNR52(u8 value)
{
	TRACE_REGISTER_WRITE

	if(value & 0x80)
	{
		if(m_hasPower == false)
		{
			m_hasPower = true;

			m_sound1->PowerOn();
			m_sound2->PowerOn();
			m_sound3->PowerOn();
			m_sound4->PowerOn();
		}
	}
	else
	{
		if(m_hasPower == true)
		{
			m_sound1->PowerOff();
			m_sound2->PowerOff();
			m_sound3->PowerOff();
			m_sound4->PowerOff();

			m_frameSequencerPosition = 0;
		}

		SetNR50(0);
		SetNR51(0);
		m_nr52 = 0x70;

		m_hasPower = false;
	}
	
	m_nr52 = (value & 0x80) | 0x70 | (m_nr52 & 0x0f);
}
