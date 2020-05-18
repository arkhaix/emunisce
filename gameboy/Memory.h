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
#ifndef MEMORY_H
#define MEMORY_H

#include "GameboyTypes.h"
#include "MachineIncludes.h"
#include "PlatformTypes.h"

namespace emunisce {

namespace WaveRamLock {
typedef int Type;

enum {
	Normal = 0,   ///< Wave RAM can be written and read normally
	SingleValue,  ///< Wave RAM can be written normally, but reads only return one value
	NoAccess,     ///< Wave RAM writes are ignored, and reads return 0xff

	NumWaveRamLockModes
};
}  // namespace WaveRamLock

namespace DmaMode {
typedef int Type;

enum {
	None = 0,  ///< No DMA currently active
	Classic,   ///< DMG DMA
	General,   ///< General purpose DMA (instant xfer)
	HBlank,    ///< H-Blank DMA (0x10 byte blocks during each hblank)

	NumDmaModes
};
}  // namespace DmaMode

class Memory : public IEmulatedMemory {
public:
	virtual ~Memory();

	virtual void SetMachine(Gameboy* machine);
	virtual void Initialize();  ///< Be sure to call the super if you override this

	virtual void Run(int ticks);

	virtual void Serialize(Archive& archive);

	void SetRegisterLocation(
		u8 registerOffset, u8* pRegister,
		bool writeable = false);  ///< registerOffset is the offset from 0xff00 (so register = 0x7e is address 0xff7e).

	u8* GetVram(int bank = -1);  ///< Returns raw pointer to vram space.  Used by Display to avoid a bunch of Read8
								 ///< calls.  bank = -1 returns the currently selected vram bank.
	u8* GetOam();  ///< Returns raw pointer to oam space.  Used by Display to avoid a bunch of Read8 calls.

	void BeginHBlank();  ///< Called from Display to facilitate HBlank DMA.
	void EndHBlank();    ///< Called from Display to facilitate HBlank DMA.

	u8 Read8(u16 address);
	u16 Read16(u16 address);

	virtual void Write8(u16 address, u8 value);
	void Write16(u16 address, u16 value);

	void SetDmaStartLocation(u8 value);
	void DisableBootRom(u8 value);
	void SetCgbRamBank(u8 value);
	void SetCgbVramBank(u8 value);

	void SetVramLock(bool locked);
	void SetOamLock(bool locked);
	void SetWaveRamLock(WaveRamLock::Type lockType,
						u8 readValue = 0xff);  ///< readValue is only necessary when locked=false.

	void SetCgbDmaSourceHigh(u8 value);
	void SetCgbDmaSourceLow(u8 value);
	void SetCgbDmaDestinationHigh(u8 value);
	void SetCgbDmaDestinationLow(u8 value);
	void CgbDmaTrigger(u8 value);

	static Memory* CreateFromFile(const char* filename);

protected:
	Memory();

	virtual void LoadBootRom(const char* filename = "dmg_rom.bin");  ///< Automatically called by Initialize
	virtual bool LoadFile(const char* filename) = 0;

	void WriteRegister(u16 address, u8 value);

	// Component pointers for handling registers.

	Gameboy* m_machine;
	EmulatedMachine::Type m_machineType;

	Cpu* m_cpu;
	Display* m_display;
	Input* m_input;
	Sound* m_sound;

	// Boot ROM

	u8 m_bootRom[0x100];
	bool m_bootRomEnabled;

	// Memory

	u8 m_memoryData[0x10000];

	u8 m_cgbRamBanks[8][0x1000];
	int m_selectedCgbRamBank;

	u8 m_cgbVramBanks[2][0x2000];
	int m_selectedCgbVramBank;

	// Registers

	u8* m_registerLocation[0x100];
	bool m_registerWriteable[0x100];
	bool m_callWriteRegister[0x100];

	// DMA

	u16 m_cgbDmaSource;
	u16 m_cgbDmaDestination;
	u8 m_cgbDmaLength;
	DmaMode::Type m_dmaMode;

	bool m_inHBlank;
	bool m_hblankDoneThisLine;

	// Display features

	bool m_vramLocked;
	bool m_oamLocked;

	// Sound features

	WaveRamLock::Type m_waveRamLockMode;
	u8 m_waveRamReadValue;
};

}  // namespace emunisce

#endif
