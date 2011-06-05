#include "sound3.h"

#include "../common/machine.h"
#include "../memory/memory.h"

#include "channelController.h"
#include "lengthUnit.h"


Sound3::Sound3()
{
	m_machine = NULL;

	m_lengthUnit->SetMaxValue(256);
}


//Sound component

void Sound3::Initialize(ChannelController* channelController)
{
	SoundGenerator::Initialize(channelController);

	SetNR30(0x7f);
	SetNR31(0xff);
	SetNR32(0x9f);
	SetNR33(0xff);
	SetNR34(0xbf);
}

void Sound3::SetMachine(Machine* machine)
{
	SoundGenerator::SetMachine(machine);

	Memory* memory = machine->GetMemory();

	memory->SetRegisterLocation(0x1a, &m_nr30, false);
	memory->SetRegisterLocation(0x1b, &m_nr31, false);
	memory->SetRegisterLocation(0x1c, &m_nr32, false);
	memory->SetRegisterLocation(0x1d, &m_nr33, false);
	memory->SetRegisterLocation(0x1e, &m_nr34, false);
}


//Sound generation

void Sound3::PowerOff()
{
	SetNR30(0);
	SetNR31(0);
	SetNR32(0);
	SetNR33(0);
	SetNR34(0);

	SoundGenerator::PowerOff();
}

void Sound3::PowerOn()
{
	SoundGenerator::PowerOn();
}


void Sound3::Run(int ticks)
{
}

float Sound3::GetSample()
{
	return 0.f;
}


//Registers

void Sound3::SetNR30(u8 value)
{
	if(m_hasPower == false)
		return;

	//Disable the DAC?
	if((value & 0x80) == 0)
	{
		m_channelController->DisableChannel();
		m_dacEnabled = false;
	}
	else
	{
		m_dacEnabled = true;
	}

	m_nr30 = value & 0x80;
	m_nr30 |= 0x7f;
}

void Sound3::SetNR31(u8 value)
{
	//DMG allows writing this even when the power is off
	//todo: CGB does not

	m_lengthUnit->WriteLengthRegister(value);

	m_nr31 = 0xff;
}

void Sound3::SetNR32(u8 value)
{
	if(m_hasPower == false)
		return;

	m_nr32 = value & 0x60;
	m_nr32 |= 0x9f;
}

void Sound3::SetNR33(u8 value)
{
	if(m_hasPower == false)
		return;

	m_nr33 = 0xff;
}

void Sound3::SetNR34(u8 value)
{
	if(m_hasPower == false)
		return;

	WriteTriggerRegister(value);

	m_nr34 = value & 0x40;
	m_nr34 |= 0xbf;
}
