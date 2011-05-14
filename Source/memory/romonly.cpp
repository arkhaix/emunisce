#include "romonly.h"

#include <cstdlib>

#include <fstream>
using namespace std;

RomOnly::RomOnly()
{
	m_hasCartRam = false;
}

void RomOnly::Initialize()
{
	//Randomize the RAM data
	for(int i=0;i<0x2000;i++)
	{
		int randomNumber = rand();

		u8 randomByte = (u8)randomNumber;
		m_cartRamData[i] = randomByte;
	}

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
	ifstream ifile(filename, ios::in | ios::binary);

	if(ifile.fail() || ifile.eof() || !ifile.good())
		return false;

	ifile.read((char*)&m_cartRomData[0], 0x8000);
	ifile.close();

	u8 cartType = m_cartRomData[0x147];
	if(cartType == 8 || cartType == 9)
		m_hasCartRam = true;

	return true;
}
