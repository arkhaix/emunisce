#ifndef MBC1_H
#define MBC1_H

#include "memory.h"

class MBC1 : public Memory
{
public:

	MBC1();

	virtual void Write8(u16 address, u8 value);

protected:

	virtual bool LoadFile(const char* filename);

	void SwitchROM();
	void SwitchRAM();

	u8 m_romBanks[0x200][0x4000];
	u8 m_ramBanks[0x10][0x2000];

	int m_selectedRomBank;
	int m_selectedRamBank;
	int m_modeSelect;	///<0 = ROM banking, 1 = RAM banking

	bool m_fiveBitBankCheck;	///<Always true for MBC1.  Disables loading into banks 0x20, 0x40, and 0x60 during LoadFile.
};

#endif
