#include "RomOnly.h"


void RomOnly::Initialize()
{
	//todo: Fill up the cart ram with random data

	Memory::Initialize();	///<Calls Reset()
}

void RomOnly::Reset()
{
	m_cartRom = m_cartRomData;
	m_switchableRom = &m_cartRomData[0x4000];
	m_switchableRam = m_cartRamData;

	Memory::Reset();
}

bool RomOnly::LoadFile(const char* filename)
{
	return false;
}
