#include "memory.h"

//CRT
#include <ctime>

//STL
#include <fstream>
#include <iostream> ///<debug
using namespace std;

//Project
#include "romonly.h"


Memory::Memory()
{
	srand(time(NULL));

	m_cartRom = NULL;
	m_switchableRom = NULL;
	m_videoRam = NULL;
	m_switchableRam = NULL;
	m_internalRam = NULL;
	m_internalRamEcho = NULL;
	m_spriteRam = NULL;
	m_stackRam = NULL;
}

Memory::~Memory()
{
}

void Memory::Initialize()
{
	//Randomize the RAM data
	for(int i=0;i<0x2000;i++)
	{
		int randomNumber = rand();
		u8 r[4];
		r[0] = (u8)(randomNumber);
		r[1] = (u8)(randomNumber >> 8);
		r[2] = (u8)(randomNumber >> 16);
		r[3] = (u8)(randomNumber >> 24);

		m_videoRamData[i] = r[0];
		m_internalRamData[i] = r[1];

		if(i < 0x100)
			m_spriteRamData[i] = r[2];

		if(i < 0x80)
			m_stackRamData[i] = r[3];
	}

	Reset();
}

void Memory::Reset()
{
	m_videoRam = m_videoRamData;
	m_internalRam = m_internalRamData;
	m_internalRamEcho = m_internalRamData;
	m_spriteRam = m_spriteRamData;
	m_stackRam = m_stackRamData;
}

void Memory::SetMachine(Machine* machine)
{
	if(machine)
	{
		machine->_Memory = this;
	}
}

u8 Memory::Read8(u16 address)
{
	u8* block = NULL;
	u16 offset = 0;

	MapMemoryFromAddress(address, &block, &offset);
	if(block == NULL)
	{
		if(IsRegisterAddress(address))
		{
			return ReadRegister(address);
		}
		else
		{
			return 0;
		}
	}

	u16 adjustedAddress = address - offset;
	return block[adjustedAddress];
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
	u8* block = NULL;
	u16 offset = 0;
	bool writeable = false;

	MapMemoryFromAddress(address, &block, &offset, &writeable);
	if(block == NULL)
	{
		if(IsRegisterAddress(address))
			WriteRegister(address, value);

		return;
	}

	if(writeable == false)
		return;

	u16 adjustedAddress = address - offset;
	block[adjustedAddress] = value;
}

void Memory::Write16(u16 address, u16 value)
{
	u8 low = (u8)(value & 0x00ff);
	Write8(address, low);

	u8 high = (u8)(value >> 8);
	Write8(address+1, high);
}
	
Memory* Memory::CreateFromFile(const char* filename)
{
	ifstream ifile(filename, ios::in | ios::binary);
	
	if(ifile.fail() || ifile.eof() || !ifile.good())
		return NULL;

	u8 header[0x150] = {0};
	ifile.read((char*)&header[0], 0x150);
	ifile.close();

	//Debug
	char title[17] = {0};
	for(int i=0;i<16;i++)
		title[i] = header[0x134+i];
	printf("Title: %s\n", title);

	/*
	0 - ROM ONLY               5 - ROM+MBC2
	1 - ROM+MBC1               6 - ROM+MBC2+BATTERY
	2 - ROM+MBC1+RAM           8 - ROM+RAM
	3 - ROM+MBC1+RAM+BATTERY   9 - ROM+RAM+BATTERY
	FF - ROM+HuC1+RAM+BATTERY
	*/
	printf("Cartridge type: ");
	u8 cartType = header[0x147];
	if(cartType == 0) printf("ROM Only\n");
	else if(cartType == 1) printf("MBC1\n");
	else if(cartType == 2) printf("MBC1 + RAM\n");
	else if(cartType == 3) printf("MBC1 + RAM + Battery\n");
	else if(cartType == 5) printf("MBC2\n");
	else if(cartType == 6) printf("MBC2 + Battery\n");
	else if(cartType == 8) printf("ROM + RAM\n");
	else if(cartType == 9) printf("ROM + RAM + Battery\n");
	else if(cartType == 0xff) printf("HuC1 + RAM + Battery\n");

	//Instantiate appropriate MBC class from header info
	Memory* memoryController = NULL;
	if(cartType == 0 || cartType == 8 || cartType == 9)
		memoryController = new RomOnly();

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

u8 Memory::ReadRegister(u16 address)
{
	return 0;
}

void Memory::WriteRegister(u16 address, u8 value)
{
}

void Memory::MapMemoryFromAddress(u16 address, u8** resultBlock, u16* offset, bool* blockIsWriteable)
{
	if(resultBlock == NULL || offset == NULL)
		return;

	if(blockIsWriteable != NULL)
		*blockIsWriteable = true;

	if(address <= 0x3fff)
	{
		*resultBlock = m_cartRom;
		*offset = 0;
		if(blockIsWriteable != NULL)
			*blockIsWriteable = false;
	}
	else if(address <= 0x7fff)
	{
		*resultBlock = m_switchableRom;
		*offset = 0x4000;
		if(blockIsWriteable != NULL)
			*blockIsWriteable = false;
	}
	else if(address <= 0x9fff)
	{
		*resultBlock = m_videoRam;
		*offset = 0x8000;
	}
	else if(address <= 0xbfff)
	{
		*resultBlock = m_switchableRam;
		*offset = 0xa000;
	}
	else if(address <= 0xdfff)
	{
		*resultBlock = m_internalRam;
		*offset = 0xc000;
	}
	else if(address <= 0xfdff)
	{
		*resultBlock = m_internalRamEcho;
		*offset = 0xe000;
	}
	else if(address <= 0xfe9f)
	{
		*resultBlock = m_spriteRam;
		*offset = 0xfe00;
	}
	else if(address <= 0xfeff)
	{
		//unusable
		*resultBlock = NULL;
		*offset = 0xfea0;
		if(blockIsWriteable != NULL)
			*blockIsWriteable = false;
	}
	else if(address <= 0xff4b)
	{
		//todo: i/o ports
		*resultBlock = NULL;
		*offset = 0xff00;
	}
	else if(address <= 0xff7f)
	{
		//unusable
		*resultBlock = NULL;
		*offset = 0xff4c;
		if(blockIsWriteable != NULL)
			*blockIsWriteable = false;
	}
	else if(address <= 0xfffe)
	{
		*resultBlock = m_stackRam;
		*offset = 0xff80;
	}
	else if(address <= 0xffff)
	{
		//todo: interrupt enable register
		*resultBlock = NULL;
		*offset = 0xffff;
	}
	else
	{
		//Missed a spot

		*resultBlock = NULL;
		*offset = 0;
		if(blockIsWriteable != NULL)
			*blockIsWriteable = false;

		volatile int x = 13;
		x /= 0;
	}
}

bool Memory::IsRegisterAddress(u16 address)
{
	if(address >= 0xff00 && address <= 0xff4b)
		return true;

	if(address == 0xffff)
		return true;

	return false;
}
