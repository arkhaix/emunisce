#include "sound.h"

#include "windows.h"	///<For critical sections.  The audio buffer needs to be locked.

#include "../common/machine.h"
#include "../memory/memory.h"

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
}

Sound::~Sound()
{
	DeleteCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
	delete (LPCRITICAL_SECTION)m_audioBufferLock;
}

void Sound::Initialize()
{
	m_nextSampleIndex = 0;

	m_audioBufferCount = 0;

	m_inaccessible = 0xff;

	SetNR52(0xf0);	///<Must enable the master control before setting the individual registers

	SetNR10(0x80);
	SetNR11(0x3f);
	SetNR12(0x00);
	SetNR13(0xff);
	SetNR14(0xbf);

	SetNR21(0x3f);
	SetNR22(0x00);
	SetNR23(0xff);
	SetNR24(0xbf);

	SetNR30(0x7f);
	SetNR31(0xff);
	SetNR32(0x9f);
	SetNR33(0xff);
	SetNR34(0xbf);

	SetNR41(0xff);
	SetNR42(0xff);
	SetNR43(0x00);
	SetNR44(0xbf);

	SetNR50(0x00);
	SetNR51(0x00);
}

void Sound::SetMachine(Machine* machine)
{
	m_machine = machine;
	m_memory = machine->GetMemory();

	m_ticksPerSample = (float)machine->GetTicksPerSecond() / (float)SamplesPerSecond;
	m_ticksUntilNextSample = m_ticksPerSample;
	m_ticksSinceLastSample = 0;


	m_memory->SetRegisterLocation(0x10, &m_nr10, false);
	m_memory->SetRegisterLocation(0x11, &m_nr11, false);
	m_memory->SetRegisterLocation(0x12, &m_nr12, false);
	m_memory->SetRegisterLocation(0x13, &m_nr13, false);
	m_memory->SetRegisterLocation(0x14, &m_nr14, false);

	m_memory->SetRegisterLocation(0x15, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x16, &m_nr21, false);
	m_memory->SetRegisterLocation(0x17, &m_nr22, false);
	m_memory->SetRegisterLocation(0x18, &m_nr23, false);
	m_memory->SetRegisterLocation(0x19, &m_nr24, false);

	m_memory->SetRegisterLocation(0x1a, &m_nr30, false);
	m_memory->SetRegisterLocation(0x1b, &m_nr31, false);
	m_memory->SetRegisterLocation(0x1c, &m_nr32, false);
	m_memory->SetRegisterLocation(0x1d, &m_nr33, false);
	m_memory->SetRegisterLocation(0x1e, &m_nr34, false);

	m_memory->SetRegisterLocation(0x1f, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x20, &m_nr41, false);
	m_memory->SetRegisterLocation(0x21, &m_nr42, false);
	m_memory->SetRegisterLocation(0x22, &m_nr43, false);
	m_memory->SetRegisterLocation(0x23, &m_nr44, false);

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
}

void Sound::Run(int ticks)
{
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

	m_nr10 = value & 0x7f;
	m_nr10 |= 0x80;
}

void Sound::SetNR11(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr11 = value & 0xc0;
	m_nr11 |= 0x3f;
}

void Sound::SetNR12(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr12 = value;
}

void Sound::SetNR13(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr13 = 0xff;
}

void Sound::SetNR14(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr14 = value & 0x40;
	m_nr14 |= 0xbf;
}

void Sound::SetNR21(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr21 = value & 0xc0;
	m_nr21 |= 0x3f;
}

void Sound::SetNR22(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr22 = value;
}

void Sound::SetNR23(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr23 = 0xff;
}

void Sound::SetNR24(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr24 = value & 0x40;
	m_nr24 |= 0xbf;
}

void Sound::SetNR30(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr30 = value & 0x80;
	m_nr30 |= 0x7f;
}

void Sound::SetNR31(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr31 = 0xff;
}

void Sound::SetNR32(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr32 = value & 0x60;
	m_nr32 |= 0x9f;
}

void Sound::SetNR33(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr33 = 0xff;
}

void Sound::SetNR34(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr34 = value & 0x40;
	m_nr34 |= 0xbf;
}

void Sound::SetNR41(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr41 = 0xff;
}

void Sound::SetNR42(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr42 = value;
}

void Sound::SetNR43(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr43 = value;
}

void Sound::SetNR44(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr44 = value & 0x40;
	m_nr44 |= 0xbf;
}

void Sound::SetNR50(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr50 = value;
}

void Sound::SetNR51(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr51 = value;
}

void Sound::SetNR52(u8 value)
{
	TRACE_REGISTER_WRITE

	if(value & 0x80)
	{
	}
	else
	{
		SetNR10(0);
		SetNR11(0);
		SetNR12(0);
		SetNR13(0);
		SetNR14(0);

		SetNR21(0);
		SetNR22(0);
		SetNR23(0);
		SetNR24(0);

		SetNR30(0);
		SetNR31(0);
		SetNR32(0);
		SetNR33(0);
		SetNR34(0);

		SetNR41(0);
		SetNR42(0);
		SetNR43(0);
		SetNR44(0);

		SetNR50(0);
		SetNR51(0);

		m_nr52 = 0x70;
	}
	
	m_nr52 = (value & 0x80) | 0x70 | (m_nr52 & 0x0f);
}
