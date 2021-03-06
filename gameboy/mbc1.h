/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

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

#include <string>

#include "memory.h"

namespace emunisce {

class Mbc1 : public Memory {
public:
	Mbc1();
	~Mbc1() override;

	void Run(int ticks) override;

	void Serialize(Archive& archive) override;

	void Write8(u16 address, u8 value) override;

protected:
	bool LoadFile(const char* filename) override;

	void SwitchROM();
	void SwitchRAM();

	void SaveRAM();
	void LoadRAM();

	void PersistRAM();

	std::string m_romFilename;
	std::string m_sramFilename;

	bool m_sramLoaded;

	int m_numRomBanks;
	static const unsigned int m_maxRomBanks = 0x200;
	u8 m_romBanks[m_maxRomBanks][0x4000];

	int m_numRamBanks;
	u8 m_ramBanks[0x10][0x2000];

	u8 m_pendingSramWrite[0x10][0x2000];
	unsigned int m_pendingSramGeneration;    ///< Incremented each time SaveRAM is called.
	unsigned int m_lastPersistedGeneration;  ///< The generation we last persisted.
	unsigned int m_lastPersistedFrameCount;  ///< The frame count the last time we persisted the sram data.

	int m_selectedRomBank;
	int m_selectedRamBank;
	int m_modeSelect;  ///< 0 = ROM banking, 1 = RAM banking

	bool m_fiveBitBankCheck;  ///< Always true for MBC1.  Disables loading into banks 0x20, 0x40, and 0x60 during
							  ///< LoadFile.
};

}  // namespace emunisce

#endif
