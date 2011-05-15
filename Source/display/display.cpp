#include "display.h"

//70224 t-states per frame (59.7fps)
//4560 t-states per v-blank (mode 01)

//80 (77-83) t-states per line in mode 10 (oam in use)
//172 (169-175) t-states per line in mode 11 (oam + vram in use)
//204 (201-207) t-states per line in mode 00 (h-blank)

Display::Display()
{
	m_activeScreenBuffer = &m_screenBuffer;
	m_stableScreenBuffer = &m_screenBuffer2;
}

void Display::SetMachine(Machine* machine)
{
	if(machine)
	{
		m_memory = machine->_Memory;
	}
}

void Display::Initialize()
{
	Reset();
}

void Display::Reset()
{
	m_activeScreenBuffer = &m_screenBuffer;
	m_stableScreenBuffer = &m_screenBuffer2;

	m_lcdControl = 0x91;
	m_lcdStatus = 0x00;	//??

	m_scrollY = 0; //??
	m_scrollX = 0; //??

	m_currentScanline = 0; //??
	m_scanlineCompare = 0; //??

	m_backgroundPalette = 0; //??
	m_spritePalette0 = 0; //??
	m_spritePalette1 = 0; //??

	m_windowX = 0; //??
	m_windowY = 0; //??
}

ScreenBuffer* Display::GetStableScreenBuffer()
{
	return m_stableScreenBuffer;
}

//0xff40 - LCDC
u8 Display::GetLcdControl()
{
	return m_lcdControl;
}

void Display::SetLcdControl(u8 value)
{
	m_lcdControl = value;
}

//0xff41 - STAT
u8 Display::GetLcdStatus()
{
	return m_lcdStatus;
}

void Display::SetLcdStatus(u8 value)
{
	value &= 0xfc;
	m_lcdStatus &= 0x03;
	m_lcdStatus |= value;
}

//0xff42 - SCY
u8 Display::GetScrollY()
{
	return m_scrollY;
}

void Display::SetScrollY(u8 value)
{
	m_scrollY = value;
}

//0xff43 - SCX
u8 Display::GetScrollX()
{
	return m_scrollX;
}

void Display::SetScrollX(u8 value)
{
	m_scrollX = value;
}

//0xff44 - LY
u8 Display::GetCurrentScanline()
{
	return m_currentScanline;
}

void Display::SetCurrentScanline(u8 value)
{
	//todo: something about resetting the counter or stopping the display
}

//0xff45 - LYC
u8 Display::GetScanlineCompare()
{
	return m_scanlineCompare;
}

void Display::SetScanlineCompare(u8 value)
{
	m_scanlineCompare = value;
	//todo: coincident flag
	//todo: interrupt
}

//0xff47 - BGP
u8 Display::GetBackgroundPalette()
{
	return m_backgroundPalette;
}

void Display::SetBackgroundPalette(u8 value)
{
	m_backgroundPalette = value;
}

//0xff48 - OBP0
u8 Display::GetSpritePalette0()
{
	return m_spritePalette0;
}

void Display::SetSpritePalette0(u8 value)
{
	m_spritePalette0 = value;
}

//0xff49 - OBP1
u8 Display::GetSpritePalette1()
{
	return m_spritePalette1;
}

void Display::SetSpritePalette1(u8 value)
{
	m_spritePalette1 = value;
}

//0xff4a - WY
u8 Display::GetWindowY()
{
	return m_windowX;
}

void Display::SetWindowY(u8 value)
{
	m_windowY = value;
}

//0xff4b - WX
u8 Display::GetWindowX()
{
	return m_windowX;
}

void Display::SetWindowX(u8 value)
{
	m_windowX = value;
}
