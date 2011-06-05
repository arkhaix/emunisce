#include "sound2.h"

#include "../common/machine.h"
#include "../memory/memory.h"


#if 0
#include <cstdio>
#define TRACE_REGISTER_WRITE printf(__FUNCTION__ "(%02X) nr52(%02X)\n", value, m_nr52);
#else
#define TRACE_REGISTER_WRITE
#endif


Sound2::Sound2()
{
	m_machine = NULL;
}


//Sound component

void Sound2::Initialize()
{
	m_nr20 = 0xff;	///<inaccessible
	SetNR21(0x3f);
	SetNR22(0x00);
	SetNR23(0xff);
	SetNR24(0xbf);
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
}

void Sound2::PowerOn()
{
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
	TRACE_REGISTER_WRITE

	m_nr21 = value & 0xc0;
	m_nr21 |= 0x3f;
}

void Sound2::SetNR22(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr22 = value;
}

void Sound2::SetNR23(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr23 = 0xff;
}

void Sound2::SetNR24(u8 value)
{
	TRACE_REGISTER_WRITE

	m_nr24 = value & 0x40;
	m_nr24 |= 0xbf;
}