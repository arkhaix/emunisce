#ifndef MEMORY_H
#define MEMORY_H

#include "../common/types.h"

class Memory
{
public:

	virtual ~Memory();

	virtual void SetMachine(Machine* machine);
	virtual void Initialize();	///<Be sure to call the super if you override this
	virtual void Reset();	///<Be sure to call the super if you override this

	void SetRegisterData(u16 address, u8* pRegister);	///<address is not offset (so use 0xff00-0xffff).

	typedef void (*TSetRegisterValue)(u16 address, u8 value);
	void SetRegisterFunction(u16 address, TSetRegisterValue function);	///<address is not offset (so use 0xff00-0xffff).

	u8 Read8(u16 address);
	u16 Read16(u16 address);

	virtual void Write8(u16 address, u8 value);
	void Write16(u16 address, u16 value);

	void SetDmaStartLocation(u8 value);

	static Memory* CreateFromFile(const char* filename);

protected:

	Memory();

	virtual bool LoadFile(const char* filename) = 0;

	u8 ReadRegister(u16 address);
	void WriteRegister(u16 address, u8 value);

	bool IsRegisterAddress(u16 address);


	//Component pointers for handling registers.

	CPU* m_cpu;
	Display* m_display;


	//Block pointers.  Set these as necessary.

	u8* m_cartRom;			//0x0000 - 0x3fff = cart rom
	u8* m_switchableRom;	//0x4000 - 0x7fff = switchable cart rom
	u8* m_videoRam;			//0x8000 - 0x9fff = video ram (handled by default)
	u8* m_switchableRam;	//0xa000 - 0xbfff = switchable cart ram
	u8* m_internalRam;		//0xc000 - 0xdfff = internal ram (handled by default)
	u8* m_internalRamEcho;	//0xe000 - 0xfdff = echo of 0xc000 - 0xddff (handled by default)
	u8* m_spriteRam;		//0xfe00 - 0xfe9f = sprite oam ram (handled by default)
							//0xfea0 - 0xfeff = unusable
	/* todo */				//0xff00 - 0xff4b = i/o ports (handled by default)
							//0xff4c - 0xff7f = unusable
	u8* m_stackRam;			//0xff80 - 0xfffe = internal ram (handled by default)
	/* todo */				//0xffff		  = interrupt enable register (handled by default)


	//Base allocated blocks.  Internal memory that is always present, regardless of cart type.

	u8 m_videoRamData[0x2000];
	u8 m_internalRamData[0x2000];
	u8 m_spriteRamData[0x100];
	u8 m_stackRamData[0x80];


	//Memory map arrays (for fast lookups)

	u8** m_memoryBlockMap[0x10000];
	int m_memoryBlockMapOffsets[0x10000];
	TSetRegisterValue m_writeRegisterFunctions[0x10000];
};

#endif
