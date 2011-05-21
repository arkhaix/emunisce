#ifndef MBC1_H
#define MBC1_H

#include "memory.h"

class MBC1 : public Memory
{
public:

	virtual void Write8(u16 address, u8 value);

protected:

	virtual bool LoadFile(const char* filename);

	void SwitchROM();
	void SwitchRAM();

	u8 m_romBanks[0x80][0x4000];
	u8 m_ramBanks[0x04][0x2000];

	int m_selectedRomBank;
	int m_selectedRamBank;
	int m_modeSelect;	///<0 = ROM banking, 1 = RAM banking
};

#endif
