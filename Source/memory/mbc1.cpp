#include "mbc1.h"

#include <fstream>
using namespace std;

MBC1::MBC1()
{
	m_fiveBitBankCheck = true;
}

void MBC1::Write8(u16 address, u8 value)
{
	//RAM Enable/Disable
	if(address < 0x2000)
	{
		//Nothing needs to be done here
		return;
	}

	//ROM Bank Select (4:0)
	else if(address < 0x4000)
	{
		value &= 0x1f;
		if(value == 0)
			value++;

		m_selectedRomBank &= ~(0x1f);
		m_selectedRomBank |= value;
		SwitchROM();
		return;
	}

	//ROM Bank Select (6:5) or RAM Bank Select (1:0)
	else if(address < 0x6000)
	{
		value &= 0x03;

		//ROM
		if(m_modeSelect == 0)
		{
			m_selectedRomBank &= 0x1f;
			m_selectedRomBank |= (value << 5);
			SwitchROM();
			return;
		}

		//RAM
		else
		{
			m_selectedRamBank = value;
			SwitchRAM();
			return;
		}
	}

	//ROM/RAM Mode Select
	else if(address < 0x8000)
	{
		value &= 0x01;
		m_modeSelect = value;
		return;
	}

	//Switchable RAM Write 
	else if(address >= 0xa000 && address < 0xc000)
	{
		//We need to save this value in addition to letting base memory handle it.
		m_ramBanks[m_selectedRamBank][address - 0xa000] = value;
		Memory::Write8(address, value);
		return;
	}

	//Special registers all return.  If we've reached this point, then we need to do a normal write.
	Memory::Write8(address, value);
}

bool MBC1::LoadFile(const char* filename)
{
	ifstream ifile(filename, ios::in | ios::binary);

	if(ifile.fail() || ifile.eof() || !ifile.good())
		return false;

	int romBank = 0;
	while(romBank < 0x80 && ifile.good() && !ifile.eof() && !ifile.fail())
	{
		if(m_fiveBitBankCheck && (romBank == 0x20 || romBank == 0x40 || romBank == 0x60))
			romBank++;

		ifile.read((char*)&m_romBanks[romBank][0], 0x4000);

		romBank++;
	}

	ifile.close();

	memcpy_s((void*)(&m_memoryData[0x0000]), 0x10000, (void*)(&m_romBanks[0][0]), 0x4000);
	memcpy_s((void*)(&m_memoryData[0x4000]), (0x10000 - 0x4000), (void*)(&m_romBanks[1][0]), 0x4000);

	return true;
}

void MBC1::SwitchROM()
{
	memcpy_s((void*)(&m_memoryData[0x4000]), (0x10000 - 0x4000), (void*)(&m_romBanks[m_selectedRomBank][0]), 0x4000);
}

void MBC1::SwitchRAM()
{
	memcpy_s((void*)(&m_memoryData[0xa000]), (0x10000 - 0xa000), (void*)(&m_ramBanks[m_selectedRamBank][0]), 0x2000);
}
