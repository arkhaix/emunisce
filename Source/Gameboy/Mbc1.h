/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

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
#ifndef MBC1_H
#define MBC1_H

#include "Memory.h"


namespace Emunisce
{

class Mbc1 : public Memory
{
public:

	Mbc1();
	~Mbc1();

	virtual void Write8(u16 address, u8 value);

protected:

	virtual bool LoadFile(const char* filename);

	void SwitchROM();
	void SwitchRAM();

	void SaveRAM();

	char m_romFilename[1024];
	char m_sramFilename[1024];

	u8 m_romBanks[0x200][0x4000];
	u8 m_ramBanks[0x10][0x2000];

	int m_selectedRomBank;
	int m_selectedRamBank;
	int m_modeSelect;	///<0 = ROM banking, 1 = RAM banking

	bool m_fiveBitBankCheck;	///<Always true for MBC1.  Disables loading into banks 0x20, 0x40, and 0x60 during LoadFile.
};

}	//namespace Emunisce

#endif
