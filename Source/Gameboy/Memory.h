/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MEMORY_H
#define MEMORY_H

#include "PlatformTypes.h"

#include "MachineIncludes.h"
#include "GameboyTypes.h"

namespace WaveRamLock
{
	typedef int Type;

	enum
	{
		Normal = 0,	///<Wave RAM can be written and read normally
		SingleValue,///<Wave RAM can be written normally, but reads only return one value
		NoAccess,	///<Wave RAM writes are ignored, and reads return 0xff

		NumWaveRamLockModes
	};
}

class Memory : public IEmulatedMemory
{
public:

	virtual ~Memory();

	virtual void SetMachine(Gameboy* machine);
	virtual void Initialize();	///<Be sure to call the super if you override this

	void SetRegisterLocation(u8 registerOffset, u8* pRegister, bool writeable=false);	///<registerOffset is the offset from 0xff00 (so register = 0x7e is address 0xff7e).

	u8 Read8(u16 address);
	u16 Read16(u16 address);

	virtual void Write8(u16 address, u8 value);
	void Write16(u16 address, u16 value);

	void SetDmaStartLocation(u8 value);
	void DisableBootRom(u8 value);

	void SetVramLock(bool locked);
	void SetOamLock(bool locked);
	void SetWaveRamLock(WaveRamLock::Type lockType, u8 readValue=0xff);	///<readValue is only necessary when locked=false.

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


	//Display features

	bool m_vramLocked;
	bool m_oamLocked;


	//Sound features

	WaveRamLock::Type m_waveRamLockMode;
	u8 m_waveRamReadValue;
};

#endif
