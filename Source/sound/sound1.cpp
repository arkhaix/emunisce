#include "sound1.h"

#include "../common/machine.h"
#include "../memory/memory.h"


Sound1::Sound1()
{
	m_machine = NULL;
}


//Sound component

void Sound1::Initialize()
{
	SetNR10(0x80);
	SetNR11(0x3f);
	SetNR12(0x00);
	SetNR13(0xff);
	SetNR14(0xbf);
}

void Sound1::SetMachine(Machine* machine)
{
	m_machine = machine;
	Memory* memory = machine->GetMemory();

	memory->SetRegisterLocation(0x10, &m_nr10, false);
	memory->SetRegisterLocation(0x11, &m_nr11, false);
	memory->SetRegisterLocation(0x12, &m_nr12, false);
	memory->SetRegisterLocation(0x13, &m_nr13, false);
	memory->SetRegisterLocation(0x14, &m_nr14, false);
}


//Sound generation

void Sound1::PowerOff()
{
	SetNR10(0);
	SetNR11(0);
	SetNR12(0);
	SetNR13(0);
	SetNR14(0);
}

void Sound1::PowerOn()
{
}


void Sound1::Run(int ticks)
{
}

float Sound1::GetSample()
{
	return 0.f;
}


//Registers

void Sound1::SetNR10(u8 value)
{
	m_nr10 = value & 0x7f;
	m_nr10 |= 0x80;
}

void Sound1::SetNR11(u8 value)
{
	m_nr11 = value & 0xc0;
	m_nr11 |= 0x3f;
}

void Sound1::SetNR12(u8 value)
{
	m_nr12 = value;
}

void Sound1::SetNR13(u8 value)
{
	m_nr13 = 0xff;
}

void Sound1::SetNR14(u8 value)
{
	m_nr14 = value & 0x40;
	m_nr14 |= 0xbf;
}
