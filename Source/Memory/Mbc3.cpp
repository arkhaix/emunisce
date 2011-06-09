#include "Mbc3.h"

Mbc3::Mbc3()
{
	m_fiveBitBankCheck = false;	///<Tells parent to allow loading into banks 0x20, 0x40, 0x60 during LoadFile.
}

void Mbc3::Write8(u16 address, u8 value)
{
	//RAM Enable/Disable
	if(address < 0x2000)
	{
		if((value & 0x0a) != 0x0a)
			SaveRAM();
	}

	//ROM Bank Select (7:0)
	else if(address < 0x4000)
	{
		value &= 0x7f;
		if(value == 0)
			value++;

		m_selectedRomBank = value;
		SwitchROM();
		return;
	}

	//RAM Bank Select (1:0) or RTC Register Select
	else if(address < 0x6000)
	{
		//RAM
		if(value <= 0x03)
		{
			m_selectedRamBank = value;
			SwitchRAM();
			return;
		}

		//RTC Register Select
		else
		{
			//todo
		}
	}

	//Latch RTC Data
	else if(address < 0x8000)
	{
		//todo
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
