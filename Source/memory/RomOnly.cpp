#include "RomOnly.h"


void RomOnly::Initialize()
{
}

void RomOnly::Reset()
{
}


u8 RomOnly::Read8(u16 address)
{
	return 0;
}

u16 RomOnly::Read16(u16 address)
{
	return 0;
}


void RomOnly::Write8(u16 address, u8 value)
{
	if(address < 0x8000)
		return;
}

void RomOnly::Write16(u16 address, u16 value)
{
	if(address < 0x8000)
		return;
}



bool RomOnly::LoadFile(const char* filename)
{
	return false;
}
