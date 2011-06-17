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
#include "Memory.h"
using namespace Emunisce;

//CRT
#include <ctime>

//STL
#include <fstream>
using namespace std;

//Project
#include "RomOnly.h"
#include "Mbc1.h"
#include "Mbc3.h"
#include "Mbc5.h"

//Solution
#include "GameboyIncludes.h"
#include "Serialization/SerializationIncludes.h"


Memory::Memory()
{
	srand((unsigned int)time(NULL));

	m_cpu = NULL;
	m_display = NULL;
	m_input = NULL;
	m_sound = NULL;

	m_oamLocked = false;
	m_vramLocked = false;
	m_waveRamLockMode = WaveRamLock::Normal;

	for(int i=0;i<0x100;i++)
	{
		m_registerLocation[i] = NULL;
		m_callWriteRegister[i] = false;
	}

	m_bootRomEnabled = false;
}

Memory::~Memory()
{
}

void Memory::SetMachine(Gameboy* machine)
{
	//Reset Machine-dependent values
	for(int i=0;i<0x100;i++)
	{
		m_registerLocation[i] = NULL;
		m_callWriteRegister[i] = false;
	}

	//Update component pointers
	m_cpu = machine->GetGbCpu();
	m_display = machine->GetGbDisplay();
	m_input = machine->GetGbInput();
	m_sound = machine->GetGbSound();

	//Update register info

	m_callWriteRegister[0x46] = true;	//Memory::SetDmaStartLocation
	m_callWriteRegister[0x50] = true;	//Memory::DisableBootRom

	m_callWriteRegister[0x04] = true;	//CPU::SetTimerDivider
	m_callWriteRegister[0x07] = true;	//CPU::SetTimerControl

	m_callWriteRegister[0x40] = true;	//Display::SetLcdControl
	m_callWriteRegister[0x41] = true;	//Display::SetLcdStatus
	m_callWriteRegister[0x44] = true;	//Display::SetCurrentScanline
	m_callWriteRegister[0x45] = true;	//Display::SetScanlineCompare

	m_callWriteRegister[0x00] = true;	//Input::SetJoypadMode

	m_callWriteRegister[0x10] = true;	//Sound::SetNR10
	m_callWriteRegister[0x11] = true;	//Sound::SetNR11
	m_callWriteRegister[0x12] = true;	//Sound::SetNR12
	m_callWriteRegister[0x13] = true;	//Sound::SetNR13
	m_callWriteRegister[0x14] = true;	//Sound::SetNR14

	m_callWriteRegister[0x16] = true;	//Sound::SetNR21
	m_callWriteRegister[0x17] = true;	//Sound::SetNR22
	m_callWriteRegister[0x18] = true;	//Sound::SetNR23
	m_callWriteRegister[0x19] = true;	//Sound::SetNR24

	m_callWriteRegister[0x1a] = true;	//Sound::SetNR30
	m_callWriteRegister[0x1b] = true;	//Sound::SetNR31
	m_callWriteRegister[0x1c] = true;	//Sound::SetNR32
	m_callWriteRegister[0x1d] = true;	//Sound::SetNR33
	m_callWriteRegister[0x1e] = true;	//Sound::SetNR34

	m_callWriteRegister[0x20] = true;	//Sound::SetNR41
	m_callWriteRegister[0x21] = true;	//Sound::SetNR42
	m_callWriteRegister[0x22] = true;	//Sound::SetNR43
	m_callWriteRegister[0x23] = true;	//Sound::SetNR44

	m_callWriteRegister[0x24] = true;	//Sound::SetNR50
	m_callWriteRegister[0x25] = true;	//Sound::SetNR51
	m_callWriteRegister[0x26] = true;	//Sound::SetNR52
}

void Memory::Initialize()
{
	//Randomize the active RAM
	for(int i=0;i<0x2000;i++)
		m_memoryData[0xa000+i] = (u8)rand();

	LoadBootRom("dmg_rom.bin");

	//Tell the cpu to skip the bootrom if there isn't one loaded
	if(m_bootRomEnabled == false && m_cpu)
		m_cpu->pc = 0x100;
}

void Memory::Serialize(Archive& archive)
{
	SerializeItem(archive, m_bootRomEnabled);


	//Memory

	for(int address = 0; address < 0x10000; address++)
		SerializeItem(archive, m_memoryData[address]);


	//Registers are set up by the components and don't need to be serialized here


	//Display features

	SerializeItem(archive, m_vramLocked);
	SerializeItem(archive, m_oamLocked);


	//Sound features

	SerializeItem(archive, m_waveRamLockMode);
	SerializeItem(archive, m_waveRamReadValue);
}

void Memory::SetRegisterLocation(u8 registerOffset, u8* pRegister, bool writeable)
{
	m_registerLocation[registerOffset] = pRegister;
	m_registerWriteable[registerOffset] = writeable;
}

u8 Memory::Read8(u16 address)
{
	if(address >= 0xff00)
	{
		u8 offset = (u8)(address & 0x00ff);
		u8* pRegister = m_registerLocation[offset];
		if(pRegister != NULL)
			return *pRegister;

		if(address >= 0xff30 && address < 0xff40)
		{
			if(m_waveRamLockMode == WaveRamLock::NoAccess)
			{
				//printf("Read(%04X):NoAccess: %02X\n", address, 0xff);
				return 0xff;
			}
			else if(m_waveRamLockMode == WaveRamLock::SingleValue)
			{
				//printf("Read(%04X):SingleValue: %02X\n", address, m_waveRamReadValue);
				return m_waveRamReadValue;
			}
			else
			{
				//printf("Read(%04X):Normal: %02X\n", address, m_memoryData[address]);
			}
		}
	}
	else if(address >= 0x8000 && address < 0xa000 && m_vramLocked)
	{
		return 0xff;
	}
	else if(address >= 0xfe00 && address < 0xfea0 && m_oamLocked)
	{
		return 0xff;
	}
	else if(address < 0x100 && m_bootRomEnabled == true)
	{
		return m_bootRom[address];
	}

	return m_memoryData[address];
}

u16 Memory::Read16(u16 address)
{
	u8 low = Read8(address);
	u8 high = Read8(address+1);

	u16 result = low | (high << 8);
	return result;
}

void Memory::Write8(u16 address, u8 value)
{
	//Read-only addresses
	if(address < 0x8000)
		return;

	//Registers
	if(address >= 0xff00)
	{
		u8 offset = (u8)(address & 0x00ff);

		if(m_callWriteRegister[offset])
			WriteRegister(address, value);

		if(m_registerWriteable[offset] && m_registerLocation[offset] != NULL)
		{
			*(m_registerLocation[offset]) = value;
			return;
		}
	}

	//Ignore writes to locked areas
	if(address >= 0x8000 && address < 0xa000 && m_vramLocked)
		return;
	if(address >= 0xfe00 && address < 0xfea0 && m_oamLocked)
		return;
	if(address >= 0xff30 && address < 0xff40 && m_waveRamLockMode == WaveRamLock::NoAccess)
		return;

	//Send notifications when applicable
	if(address >= 0x8000 && address < 0xa000)
		m_display->WriteVram(address, value);
	if(address >= 0xfe00 && address < 0xfea0)
		m_display->WriteOam(address, value);

	//[$e000,$fdff] is mirrored to [$c000,$ddff]
	if(address >= 0xe000 && address < 0xfe00)
		Write8(address - 0x2000, value);

	//Write it
	m_memoryData[address] = value;
}

void Memory::Write16(u16 address, u16 value)
{
	u8 low = (u8)(value & 0x00ff);
	Write8(address, low);

	u8 high = (u8)(value >> 8);
	Write8(address+1, high);
}

void Memory::SetDmaStartLocation(u8 value)
{
	u16 startAddress = value * 0x0100;

	//Some games use DMA during modes 10 and 11, and expect it to work.
	//I'm not sure what really goes on yet in the hardware, but for now I'm allowing DMA to bypass the OAM lock.
	bool preserveOamLock = m_oamLocked;
	m_oamLocked = false;

	for(int i=0;i<0xa0;i++)
	{
		u16 sourceAddress = startAddress + i;
		u16 targetAddress = 0xfe00 + i;

		u8 value = Read8(sourceAddress);
		Write8(targetAddress, value);
	}

	m_oamLocked = preserveOamLock;
}

void Memory::DisableBootRom(u8 value)
{
	if(m_bootRomEnabled == false)
		return;

	if(value != 0x01)
		return;

	m_bootRomEnabled = false;
}

void Memory::SetVramLock(bool locked)
{
	m_vramLocked = locked;
}

void Memory::SetOamLock(bool locked)
{
	m_oamLocked = locked;
}

void Memory::SetWaveRamLock(WaveRamLock::Type lockType, u8 readValue)
{
	m_waveRamLockMode = lockType;
	m_waveRamReadValue = readValue;
}
	
Memory* Memory::CreateFromFile(const char* filename)
{
	//Read the gb cart header out of the file
	ifstream ifile(filename, ios::in | ios::binary);
	
	if(ifile.fail() || ifile.eof() || !ifile.good())
		return NULL;

	u8 header[0x150] = {0};
	ifile.read((char*)&header[0], 0x150);
	ifile.close();

	
	//Instantiate the appropriate MBC class from the header info
	Memory* memoryController = NULL;
	u8 cartType = header[0x147];

	if(cartType == 0 || cartType == 8 || cartType == 9)
		memoryController = new RomOnly();
	else if(cartType >= 1 && cartType <= 3)
		memoryController = new Mbc1();
	else if(cartType >= 0x0f && cartType <= 0x13)
		memoryController = new Mbc3();
	else if(cartType >= 0x19 && cartType <= 0x1e)
		memoryController = new Mbc5();

	if(memoryController != NULL)
		printf("Cartridge type ok: %d (0x%02X)\n", cartType, cartType);
	else
		printf("Unsupported cartridge type: %d (0x%02X)\n", cartType, cartType);

	if(memoryController == NULL)
		return NULL;


	//Have the MBC class load the file
	if(memoryController->LoadFile(filename) == false)
	{
		delete memoryController;
		return NULL;
	}

	return memoryController;
}

void Memory::LoadBootRom(const char* filename)
{
	m_bootRomEnabled = false;

	ifstream bootRom;
	bootRom.open(filename, ios::in | ios::binary);

	if(bootRom.eof() || bootRom.fail() || !bootRom.good())
	{
		//Use default boot rom
		u8 defaultBootRom[] = 
		{
			//Enable the lcd
			0x3e, 0x91,			//LD A,$91
			0xe0, 0x40,			//LD ($FF00+$40),A

			//Disable boot rom area
			0x3e, 0x01,			//LD A, $01
			0xe0, 0x50,			//LD ($FF50), A
		};

		memset((void*)&m_bootRom[0], 0x00, 0x100);	///<Fill it with NOPs

		//Copy the default boot rom to the end of the boot rom space (so that it finishes at 0x100 where the cart starts)
		int beginAddress = 0x100 - sizeof(defaultBootRom);
		memcpy_s((void*)(&m_bootRom[beginAddress]), 0x100-beginAddress, (void*)(&defaultBootRom[0]), sizeof(defaultBootRom));

		m_bootRomEnabled = true;
		return;
	}

	//Use the real boot rom
	bootRom.read((char*)(&m_bootRom[0]), 0x100);
	bootRom.close();

	m_bootRomEnabled = true;
}

void Memory::WriteRegister(u16 address, u8 value)
{
	switch(address)
	{
	case 0xff00: m_input->SetJoypadMode(value); break;

	case 0xff04: m_cpu->SetTimerDivider(value); break;
	case 0xff07: m_cpu->SetTimerControl(value); break;

	case 0xff10: m_sound->SetNR10(value); break;
	case 0xff11: m_sound->SetNR11(value); break;
	case 0xff12: m_sound->SetNR12(value); break;
	case 0xff13: m_sound->SetNR13(value); break;
	case 0xff14: m_sound->SetNR14(value); break;

	case 0xff16: m_sound->SetNR21(value); break;
	case 0xff17: m_sound->SetNR22(value); break;
	case 0xff18: m_sound->SetNR23(value); break;
	case 0xff19: m_sound->SetNR24(value); break;

	case 0xff1a: m_sound->SetNR30(value); break;
	case 0xff1b: m_sound->SetNR31(value); break;
	case 0xff1c: m_sound->SetNR32(value); break;
	case 0xff1d: m_sound->SetNR33(value); break;
	case 0xff1e: m_sound->SetNR34(value); break;

	case 0xff20: m_sound->SetNR41(value); break;
	case 0xff21: m_sound->SetNR42(value); break;
	case 0xff22: m_sound->SetNR43(value); break;
	case 0xff23: m_sound->SetNR44(value); break;
	
	case 0xff24: m_sound->SetNR50(value); break;
	case 0xff25: m_sound->SetNR51(value); break;
	case 0xff26: m_sound->SetNR52(value); break;

	case 0xff40: m_display->SetLcdControl(value); break;
	case 0xff41: m_display->SetLcdStatus(value); break;
	case 0xff44: m_display->SetCurrentScanline(value); break;
	case 0xff45: m_display->SetScanlineCompare(value); break;

	case 0xff46: SetDmaStartLocation(value); break;

	case 0xff50: DisableBootRom(value); break;
	}
}

