#include "memory.h"

//CRT
#include <ctime>

//STL
#include <fstream>
using namespace std;

//Project
#include "romonly.h"

//Solution
#include "../cpu/cpu.h"
#include "../display/display.h"


Memory::Memory()
{
	srand(time(NULL));

	m_cpu = NULL;
	m_display = NULL;

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

void Memory::SetMachine(Machine* machine)
{
	if(machine)
	{
		m_cpu = machine->_CPU;
		m_display = machine->_Display;
	}
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

	//Send notifications when applicable
	if(address >= 0x8000 && address < 0xa000 && m_display)
		m_display->WriteVram(address, value);
	else if(address >= 0xfe00 && address < 0xfea0 && m_display)
		m_display->WriteOam(address, value);

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

void Memory::SetDmaStartLocation(u8 value)
{
	u16 startAddress = value * 0x0100;

	for(int i=0;i<0xa0;i++)
	{
		u16 sourceAddress = startAddress + i;
		u16 targetAddress = 0xfe00 + i;

		u8 value = Read8(sourceAddress);
		Write8(targetAddress, value);
	}
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
	switch(address)
	{
	case 0xff0f: if(m_cpu) { return m_cpu->GetInterruptFlags(); } break;
	case 0xff40: if(m_display) { return m_display->GetLcdControl(); } break;
	case 0xff41: if(m_display) { return m_display->GetLcdStatus(); } break;
	case 0xff42: if(m_display) { return m_display->GetScrollY(); } break;
	case 0xff43: if(m_display) { return m_display->GetScrollX(); } break;
	case 0xff44: if(m_display) { return m_display->GetCurrentScanline(); } break;
	case 0xff45: if(m_display) { return m_display->GetScanlineCompare(); } break;
	//case 0xff46: DMA, write-only
	case 0xff47: if(m_display) { return m_display->GetBackgroundPalette(); } break;
	case 0xff48: if(m_display) { return m_display->GetSpritePalette0(); } break;
	case 0xff49: if(m_display) { return m_display->GetSpritePalette1(); } break;
	case 0xff4a: if(m_display) { return m_display->GetWindowY(); } break;
	case 0xff4b: if(m_display) { return m_display->GetWindowX(); } break;
	case 0xffff: if(m_cpu) { return m_cpu->GetInterruptsEnabled(); } break;
	}

	return 0;
}

void Memory::WriteRegister(u16 address, u8 value)
{
	switch(address)
	{
	case 0xff0f: if(m_cpu) { m_cpu->SetInterruptFlags(value); } break;
	case 0xff40: if(m_display) { m_display->SetLcdControl(value); } break;
	case 0xff41: if(m_display) { m_display->SetLcdStatus(value); } break;
	case 0xff42: if(m_display) { m_display->SetScrollY(value); } break;
	case 0xff43: if(m_display) { m_display->SetScrollX(value); } break;
	case 0xff44: if(m_display) { m_display->SetCurrentScanline(value); } break;
	case 0xff45: if(m_display) { m_display->SetScanlineCompare(value); } break;
	case 0xff46: SetDmaStartLocation(value); break;
	case 0xff47: if(m_display) { m_display->SetBackgroundPalette(value); } break;
	case 0xff48: if(m_display) { m_display->SetSpritePalette0(value); } break;
	case 0xff49: if(m_display) { m_display->SetSpritePalette1(value); } break;
	case 0xff4a: if(m_display) { m_display->SetWindowY(value); } break;
	case 0xff4b: if(m_display) { m_display->SetWindowX(value); } break;
	case 0xffff: if(m_cpu) { m_cpu->SetInterruptsEnabled(value); } break;
	}
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
