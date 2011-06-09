#include "Mbc1.h"

#include <fstream>
using namespace std;

MBC1::MBC1()
{
	m_fiveBitBankCheck = true;
}

MBC1::~MBC1()
{
}

void MBC1::Write8(u16 address, u8 value)
{
	//RAM Enable/Disable
	if(address < 0x2000)
	{
		if((value & 0x0a) != 0x0a)
			SaveRAM();

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
	//Open the file

	ifstream ifile(filename, ios::in | ios::binary);

	if(ifile.fail() || ifile.eof() || !ifile.good())
		return false;


	//Save the filename

	strcpy_s(m_romFilename, 1024, filename);
	strcpy_s(m_sramFilename, 1024, filename);
	strcat_s(m_sramFilename, 1024, ".sram");


	//Load all the banks

	int romBank = 0;
	while(romBank < 0x80 && ifile.good() && !ifile.eof() && !ifile.fail())
	{
		if(m_fiveBitBankCheck && (romBank == 0x20 || romBank == 0x40 || romBank == 0x60))
			romBank++;

		ifile.read((char*)&m_romBanks[romBank][0], 0x4000);

		romBank++;
	}

	ifile.close();


	//Copy the first two ROM banks to active memory

	memcpy_s((void*)(&m_memoryData[0x0000]), 0x10000, (void*)(&m_romBanks[0][0]), 0x4000);

	m_selectedRomBank = 1;
	SwitchROM();


	//Randomize the RAM banks

	for(u16 address=0;address<0x2000;address++)
	{
		int randomNumber = rand();
		u8 randomByte[4] = { (u8)(randomNumber>>24), (u8)(randomNumber>>16), (u8)(randomNumber>>8), (u8)randomNumber };

		m_ramBanks[0][address] = randomByte[0];
		m_ramBanks[1][address] = randomByte[1];
		m_ramBanks[2][address] = randomByte[2];
		m_ramBanks[3][address] = randomByte[3];
	}


	//Load battery-backed RAM if applicable

	ifstream sramFile(m_sramFilename, ios::in | ios::binary);
	if(sramFile.good())
	{
		for(int i=0;i<0x10;i++)
			sramFile.read((char*)(&m_ramBanks[i][0]), 0x2000);
		sramFile.close();
	}



	//Copy the first RAM bank to active memory

	m_selectedRamBank = 0;
	SwitchRAM();


	//Done

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

void MBC1::SaveRAM()
{
	u8 cartType = m_memoryData[0x147];
	u8 batteryTypes[] = { 0x03, 0x06, 0x09, 0x0f, 0x10, 0x13, 0x1b, 0 };

	bool isBatteryType = false;
	for(u8* batteryType = batteryTypes; *batteryType != 0; batteryType++)
		if(*batteryType == cartType)
			isBatteryType = true;

	if(isBatteryType == false)
		return;
	
	ofstream sramFile(m_sramFilename, ios::out | ios::binary);
	if(sramFile.good())
	{
		for(int i=0;i<0x10;i++)
			sramFile.write((char*)(&m_ramBanks[i][0]), 0x2000);
		sramFile.close();
	}
}
