#include "sound4.h"

#include "../common/machine.h"
#include "../memory/memory.h"


#if 0
#include <cstdio>
#define TRACE_REGISTER_WRITE printf(__FUNCTION__ "(%02X) nr52(%02X)\n", value, m_nr52);
#else
#define TRACE_REGISTER_WRITE
#endif


Sound4::Sound4()
{
	m_machine = NULL;
}


//Sound component

void Sound4::Initialize()
{
	m_nr40 = 0xff;	///<inaccessible
	SetNR41(0xff);
	SetNR42(0xff);
	SetNR43(0x00);
	SetNR44(0xbf);
}

void Sound4::SetMachine(Machine* machine)
{
	m_machine = machine;
	Memory* memory = machine->GetMemory();

	memory->SetRegisterLocation(0x1f, &m_nr40, false);
	memory->SetRegisterLocation(0x20, &m_nr41, false);
	memory->SetRegisterLocation(0x21, &m_nr42, false);
	memory->SetRegisterLocation(0x22, &m_nr43, false);
	memory->SetRegisterLocation(0x23, &m_nr44, false);
}


//Sound generation

void Sound4::PowerOff()
{
	SetNR41(0);
	SetNR42(0);
	SetNR43(0);
	SetNR44(0);
}

void Sound4::PowerOn()
{
}


void Sound4::Run(int ticks)
{
}

float Sound4::GetSample()
{
	return 0.f;
}


//Registers

void Sound4::SetNR41(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr41 = 0xff;
}

void Sound4::SetNR42(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr42 = value;
}

void Sound4::SetNR43(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr43 = value;
}

void Sound4::SetNR44(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr44 = value & 0x40;
	m_nr44 |= 0xbf;
}
