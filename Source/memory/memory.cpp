#include "memory.h"

//CRT
#include <ctime>

//STL
#include <fstream>
using namespace std;

//Project
#include "romonly.h"
#include "mbc1.h"
#include "mbc3.h"
#include "mbc5.h"

//Solution
#include "../common/machine.h"
#include "../cpu/cpu.h"
#include "../display/display.h"
#include "../input/input.h"
#include "../sound/sound.h"


Memory::Memory()
{
	srand(time(NULL));

	m_cpu = NULL;
	m_display = NULL;
	m_input = NULL;
	m_sound = NULL;

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

void Memory::SetMachine(Machine* machine)
{
	//Reset machine-dependent values
	for(int i=0;i<0x100;i++)
	{
		m_registerLocation[i] = NULL;
		m_callWriteRegister[i] = false;
	}

	//Update component pointers
	m_cpu = machine->GetCpu();
	m_display = machine->GetDisplay();
	m_input = machine->GetInput();
	m_sound = machine->GetSound();

	//Update register info

	m_callWriteRegister[0x46] = true;	//Memory::SetDmaStartLocation
	m_callWriteRegister[0x50] = true;	//Memory::DisableBootRom

	m_callWriteRegister[0x04] = true;	//CPU::SetTimerDivider
	m_callWriteRegister[0x07] = true;	//CPU::SetTimerControl

	m_callWriteRegister[0x44] = true;	//Display::SetCurrentScanline
	m_callWriteRegister[0x45] = true;	//Display::SetScanlineCompare

	m_callWriteRegister[0x00] = true;	//Input::SetJoypadMode

	m_callWriteRegister[0x10] = true;	//Sound::SetNR10
	m_callWriteRegister[0x11] = true;	//Sound::SetNR11
	m_callWriteRegister[0x12] = true;	//Sound::SetNR12
	m_callWriteRegister[0x13] = true;	//Sound::SetNR13
	m_callWriteRegister[0x14] = true;	//Sound::SetNR14
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

	if(address < 0x100 && m_bootRomEnabled == true)
		return m_bootRom[address];

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

void Memory::DisableBootRom(u8 value)
{
	if(m_bootRomEnabled == false)
		return;

	if(value != 0x01)
		return;

	m_bootRomEnabled = false;
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
		memoryController = new MBC1();
	else if(cartType >= 0x0f && cartType <= 0x13)
		memoryController = new MBC3();
	else if(cartType >= 0x19 && cartType <= 0x1e)
		memoryController = new MBC5();

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
	case 0xff44: m_display->SetCurrentScanline(value); break;
	case 0xff45: m_display->SetScanlineCompare(value); break;
	case 0xff46: SetDmaStartLocation(value); break;
	case 0xff50: DisableBootRom(value); break;
	}
}

