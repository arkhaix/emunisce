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

	for(int i=0;i<0x100;i++)
	{
		m_registerLocation[i] = NULL;
		m_callWriteRegister[i] = false;
	}
}

Memory::~Memory()
{
}

void Memory::SetMachine(Machine* machine)
{
	//Reset machine-dependent values
	for(int i=0;i<0x100;i++)
	{
		m_registerLocation[i] = NULL;
		m_callWriteRegister[i] = false;
	}

	//Update component pointers
	if(machine)
	{
		m_cpu = machine->_CPU;
		m_display = machine->_Display;
	}

	//Update register info

	m_callWriteRegister[0x46] = true;

	if(m_display)
	{
		m_callWriteRegister[0x40] = true;
		m_callWriteRegister[0x41] = true;
		m_callWriteRegister[0x42] = true;
		m_callWriteRegister[0x43] = true;
		m_callWriteRegister[0x44] = true;
		m_callWriteRegister[0x45] = true;
		m_callWriteRegister[0x47] = true;
		m_callWriteRegister[0x48] = true;
		m_callWriteRegister[0x49] = true;
		m_callWriteRegister[0x4a] = true;
		m_callWriteRegister[0x4b] = true;
	}
}

void Memory::Initialize()
{
	Reset();
}

void Memory::Reset()
{
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

	//Send notifications when applicable
	if(address >= 0x8000 && address < 0xa000 && m_display)
		m_display->WriteVram(address, value);
	else if(address >= 0xfe00 && address < 0xfea0 && m_display)
		m_display->WriteOam(address, value);

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

void Memory::WriteRegister(u16 address, u8 value)
{
	switch(address)
	{
	case 0xff44: if(m_display) { m_display->SetCurrentScanline(value); } break;
	case 0xff45: if(m_display) { m_display->SetScanlineCompare(value); } break;
	case 0xff46: SetDmaStartLocation(value); break;
	}
}

