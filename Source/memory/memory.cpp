#include "memory.h"

Memory::Memory()
{
	m_cartRom = NULL;
	m_switchableRom = NULL;
	m_videoRam = NULL;
	m_switchableRam = NULL;
	m_internalRam = NULL;
	m_internalRamEcho = NULL;
	m_spriteRam = NULL;
	m_zeroPage = NULL;
}

Memory::~Memory()
{
}

void Memory::Initialize()
{
	//todo: Fill the blocks with random data

	Reset();
}

void Memory::Reset()
{
	m_videoRam = m_videoRamData;
	m_internalRam = m_internalRamData;
	m_internalRamEcho = m_internalRamData;
	m_spriteRam = m_spriteRamData;
	m_zeroPage = m_zeroPageData;
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
	
Memory* Memory::CreateFromFile(Machine* machine, const char* filename)
{
	//Open file

	//Read header

	//Close the file

	//Instantiate appropriate MBC class from header info
	Memory* memoryController = NULL;

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
		*resultBlock = m_zeroPage;
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
