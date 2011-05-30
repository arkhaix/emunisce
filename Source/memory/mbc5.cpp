#include "mbc5.h"

MBC5::MBC5()
{
	m_fiveBitBankCheck = false;	///<Tells parent to allow loading into banks 0x20, 0x40, 0x60 during LoadFile.
}

void MBC5::Write8(u16 address, u8 value)
{
	//RAM Enable/Disable
	if(address < 0x2000)
	{
		//Nothing needs to be done here
		return;
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
