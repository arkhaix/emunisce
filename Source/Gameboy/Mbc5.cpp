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
#include "Mbc5.h"
using namespace Emunisce;

Mbc5::Mbc5()
{
	m_fiveBitBankCheck = false;	///<Tells parent to allow loading into banks 0x20, 0x40, 0x60 during LoadFile.
}

void Mbc5::Write8(u16 address, u8 value)
{
	//RAM Enable/Disable
	if(address < 0x2000)
	{
		if((value & 0x0a) != 0x0a)
			SaveRAM();
		else if(m_sramLoaded == false)
			LoadRAM();
	}

	//ROM Bank Select (low 8 bits)
	else if(address < 0x3000)
	{
		m_selectedRomBank &= ~(0xff);
		m_selectedRomBank |= value;
		SwitchROM();
		return;
	}

	//ROM Bank Select (high 1 bit)
	else if(address < 0x4000)
	{
		value &= 0x01;

		m_selectedRomBank &= ~(0x100);
		m_selectedRomBank |= (value << 8);
		SwitchROM();
		return;
	}

	//RAM Bank Select (3:0)
	else if(address < 0x6000)
	{
		value &= 0x0f;
		m_selectedRamBank = value;
		SwitchRAM();
		return;
	}

	//Switchable RAM Write 
	else if(address >= 0xa000 && address < 0xc000)
	{
		//We need to save this value in addition to letting base memory handle it.
		m_ramBanks[m_selectedRamBank][address - 0xa000] = value;
		Memory::Write8(address, value);
		return;
	}

	//Special registers all return.  If we've reached this point, then we need to do a normal write.
	Memory::Write8(address, value);
}
