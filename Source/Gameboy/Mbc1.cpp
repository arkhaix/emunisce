/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Mbc1.h"
using namespace Emunisce;

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory.h>
using namespace std;

#include "Serialization/SerializationIncludes.h"

#include "Gameboy.h"


Mbc1::Mbc1()
{
	m_fiveBitBankCheck = true;

	m_numRomBanks = 0;
	m_numRamBanks = 0;

	m_sramLoaded = false;
}

Mbc1::~Mbc1()
{
}

void Mbc1::Serialize(Archive& archive)
{
	Memory::Serialize(archive);

	//ROM banks will be loaded from the cart and can't change, so just save the ram banks
	SerializeBuffer(archive, &m_ramBanks[0][0], 0x2000 * m_numRamBanks);

	SerializeItem(archive, m_selectedRomBank);
	SerializeItem(archive, m_selectedRamBank);
	SerializeItem(archive, m_modeSelect);

	if(archive.GetArchiveMode() == ArchiveMode::Loading)
	{
		//Memory doesn't save out the ROM area, so we have to restore the bank-switched part
		SwitchROM();
	}
}

void Mbc1::Write8(u16 address, u8 value)
{
	//RAM Enable/Disable
	if(address < 0x2000)
	{
		if((value & 0x0a) != 0x0a)
			SaveRAM();
		else if(m_sramLoaded == false)
			LoadRAM();

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

bool Mbc1::LoadFile(const char* filename)
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

	memcpy((void*)(&m_memoryData[0x0000]), (void*)(&m_romBanks[0][0]), 0x4000);

	m_selectedRomBank = 1;
	SwitchROM();


	//Get the relevant cartridge header info

	u8 romSize = m_memoryData[0x0148];
	if(romSize == 0)
		m_numRomBanks = 0;
	else if(romSize < 8)
		m_numRomBanks = 2 << romSize;
	else if(romSize == 0x52)
		m_numRomBanks = 72;
	else if(romSize == 0x53)
		m_numRomBanks = 80;
	else if(romSize == 0x54)
		m_numRomBanks = 96;
	else
		m_numRomBanks = 0;

	u8 ramSize = m_memoryData[0x0149];
	if(ramSize <= 1)
		m_numRamBanks = 0;
	else if(ramSize == 2)
		m_numRamBanks = 4;
	else if(ramSize == 3)
		m_numRamBanks = 16;


	//Randomize the RAM banks

	for(int i=0;i<m_numRamBanks;i++)
		for(u16 address=0;address<0x2000;address++)
			m_ramBanks[i][address] = rand();


	//Copy the first RAM bank to active memory

	m_selectedRamBank = 0;
	SwitchRAM();


	//Done

	return true;
}

void Mbc1::SwitchROM()
{
	memcpy((void*)(&m_memoryData[0x4000]), (void*)(&m_romBanks[m_selectedRomBank][0]), 0x4000);
}

void Mbc1::SwitchRAM()
{
	memcpy((void*)(&m_memoryData[0xa000]), (void*)(&m_ramBanks[m_selectedRamBank][0]), 0x2000);
}

void Mbc1::SaveRAM()
{
	u8 cartType = m_memoryData[0x147];
	u8 batteryTypes[] = { 0x03, 0x06, 0x09, 0x0f, 0x10, 0x13, 0x1b, 0 };

	bool isBatteryType = false;
	for(u8* batteryType = batteryTypes; *batteryType != 0; batteryType++)
		if(*batteryType == cartType)
			isBatteryType = true;

	if(isBatteryType == false)
		return;
	
	IMachineToApplication* applicationInterface = m_machine->GetApplicationInterface();
	if(applicationInterface != NULL)
	{
		applicationInterface->SaveRomData("sram", &m_ramBanks[0][0], 0x2000 * m_numRamBanks);
		m_sramLoaded = true;	///<So we don't wind up re-loading this data we just wrote
	}
}

void Mbc1::LoadRAM()
{
	if(m_sramLoaded == true)
		return;

	u8 cartType = m_memoryData[0x147];
	u8 batteryTypes[] = { 0x03, 0x06, 0x09, 0x0f, 0x10, 0x13, 0x1b, 0 };

	bool isBatteryType = false;
	for(u8* batteryType = batteryTypes; *batteryType != 0; batteryType++)
		if(*batteryType == cartType)
			isBatteryType = true;

	if(isBatteryType == false)
		return;

	IMachineToApplication* applicationInterface = m_machine->GetApplicationInterface();
	if(applicationInterface != NULL)
	{
		applicationInterface->LoadRomData("sram", &m_ramBanks[0][0], 0x2000 * m_numRamBanks);
		m_sramLoaded = true;
		SwitchRAM();
	}
}
