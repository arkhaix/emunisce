#include "sound2.h"

#include "../common/machine.h"
#include "../memory/memory.h"


Sound2::Sound2()
{
	m_machine = NULL;

	m_lengthCounterMaxValue = 64;
}


//Sound component

void Sound2::Initialize(ChannelController* channelController)
{
	m_nr20 = 0xff;	///<inaccessible
	SetNR21(0x3f);
	SetNR22(0x00);
	SetNR23(0xff);
	SetNR24(0xbf);

	SoundGenerator::Initialize(channelController);
}

void Sound2::SetMachine(Machine* machine)
{
	m_machine = machine;
	Memory* memory = machine->GetMemory();

	memory->SetRegisterLocation(0x15, &m_nr20, false);
	memory->SetRegisterLocation(0x16, &m_nr21, false);
	memory->SetRegisterLocation(0x17, &m_nr22, false);
	memory->SetRegisterLocation(0x18, &m_nr23, false);
	memory->SetRegisterLocation(0x19, &m_nr24, false);
}


//Sound generation

void Sound2::PowerOff()
{
	SetNR21(0);
	SetNR22(0);
	SetNR23(0);
	SetNR24(0);

	SoundGenerator::PowerOff();
}

void Sound2::PowerOn()
{
	SoundGenerator::PowerOn();
}


void Sound2::Run(int ticks)
{
}

float Sound2::GetSample()
{
	return 0.f;
}


//Registers

void Sound2::SetNR21(u8 value)
{
	//DMG allows writing this even when the power is off
	//todo: CGB does not

	if(m_hasPower == true)
	{
		m_nr21 = value & 0xc0;
	}

	WriteLengthRegister(value & 0x3f);

	m_nr21 |= 0x3f;
}

void Sound2::SetNR22(u8 value)
{
	if(m_hasPower == false)
		return;

	m_nr22 = value;
}

void Sound2::SetNR23(u8 value)
{
	if(m_hasPower == false)
		return;

	m_nr23 = 0xff;
}

void Sound2::SetNR24(u8 value)
{
	if(m_hasPower == false)
		return;

	m_nr24 = value & 0x40;
	m_nr24 |= 0xbf;
}
