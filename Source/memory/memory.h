#ifndef MEMORY_H
#define MEMORY_H

#include "../common/types.h"

class Memory
{
public:

	virtual ~Memory();

	virtual void SetMachine(Machine* machine);
	virtual void Initialize();	///<Be sure to call the super if you override this

	void SetRegisterLocation(u8 registerOffset, u8* pRegister, bool writeable=false);	///<registerOffset is the offset from 0xff00 (so register = 0x7e is address 0xff7e).

	u8 Read8(u16 address);
	u16 Read16(u16 address);

	virtual void Write8(u16 address, u8 value);
	void Write16(u16 address, u16 value);

	void SetDmaStartLocation(u8 value);
	void DisableBootRom(u8 value);

	static Memory* CreateFromFile(const char* filename);

protected:

	Memory();

	virtual void LoadBootRom(const char* filename = "dmg_rom.bin");	///<Automatically called by Initialize
	virtual bool LoadFile(const char* filename) = 0;

	void WriteRegister(u16 address, u8 value);


	//Component pointers for handling registers.

	Cpu* m_cpu;
	Display* m_display;
	Input* m_input;
	Sound* m_sound;


	//Boot ROM

	u8 m_bootRom[0x100];
	bool m_bootRomEnabled;


	//Memory

	u8 m_memoryData[0x10000];


	//Registers

	u8* m_registerLocation[0x100];
	bool m_registerWriteable[0x100];
	bool m_callWriteRegister[0x100];
};

#endif
