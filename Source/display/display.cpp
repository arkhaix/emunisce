#include "windows.h"	///<For critical sections.  We need to lock the screen buffer.

#include "display.h"

#include "../memory/memory.h"


//70224 t-states per frame (59.7fps)
//4560 t-states per v-blank (mode 01)

//80 (77-83) t-states per line in mode 10 (oam in use)
//172 (169-175) t-states per line in mode 11 (oam + vram in use)
//204 (201-207) t-states per line in mode 00 (h-blank)


//Bit defines for LCDC (0xff40)
#define LCDC_Background	(1<<0)
#define LCDC_Sprites	(1<<1)
#define LCDC_SpriteSize	(1<<2)
#define LCDC_BackgroundTileMap	(1<<3)
#define LCDC_TileData	(1<<4)
#define LCDC_Window		(1<<5)
#define LCDC_WindowTileMap	(1<<6)
#define LCDC_Control	(1<<7)

//Bit defines for STAT (0xff41)
#define STAT_Mode	(3<<0)
#define STAT_Coincidence	(1<<2)
#define STAT_Interrupt_HBlank	(1<<3)
#define STAT_Interrupt_VBlank	(1<<4)
#define STAT_Interrupt_SpriteLock	(1<<5)
#define STAT_Interrupt_Coincidence	(1<<6)

//Values for STAT_Mode
#define Mode_HBlank	(0)
#define Mode_VBlank	(1)
#define Mode_SpriteLock	(2)
#define Mode_VideoRamLock	(3)


Display::Display()
{
	m_activeScreenBuffer = &m_screenBuffer;
	m_stableScreenBuffer = &m_screenBuffer2;

	m_screenBufferLock = (void*)(new CRITICAL_SECTION());
	InitializeCriticalSection((LPCRITICAL_SECTION)m_screenBufferLock);
}

Display::~Display()
{
	DeleteCriticalSection((LPCRITICAL_SECTION)m_screenBufferLock);
	delete (LPCRITICAL_SECTION)m_screenBufferLock;
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

	//Start everything off at 0,0
	m_currentScanline = 153;
	m_stateTicksRemaining = 0;
	m_vblankScanlineTicksRemaining = 0;
	Begin_SpritesLocked();
}

void Display::Run(int ticks)
{
	if(m_currentState == DisplayState::VBlank)
		Run_VBlank(ticks);

	m_stateTicksRemaining -= ticks;
	if(m_stateTicksRemaining <= 0)
	{
		if(m_currentState == DisplayState::VBlank)
			Begin_SpritesLocked();
		else if(m_currentState == DisplayState::SpritesLocked)
			Begin_VideoRamLocked();
		else if(m_currentState == DisplayState::VideoRamLocked)
			Begin_HBlank();
		else //m_currentState == DisplayState::HBlank
			Begin_SpritesLocked();	///<This will trigger VBlank when appropriate
	}
}

ScreenBuffer Display::GetStableScreenBuffer()
{
	EnterCriticalSection( (LPCRITICAL_SECTION)m_screenBufferLock );
		ScreenBuffer result = *m_stableScreenBuffer;
	LeaveCriticalSection( (LPCRITICAL_SECTION)m_screenBufferLock );

	return result;
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
	value &= ~(STAT_Mode);		//Erase incoming STAT_Mode
	m_lcdStatus &= STAT_Mode;	//Preserve only the existing STAT_Mode
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

void Display::Begin_HBlank()
{
	m_currentState = DisplayState::HBlank;
	m_stateTicksRemaining += 204;

	RenderScanline();

	//Set mode 00
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_HBlank;

	//todo: interrupt
}

void Display::Begin_VBlank()
{
	m_currentState = DisplayState::VBlank;
	m_stateTicksRemaining += 4560;

	m_currentScanline = 144;
	//todo: coincident flag
	//todo: interrupt

	m_vblankScanlineTicksRemaining = m_stateTicksRemaining % 456;

	//Set mode 01
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_VBlank;

	//todo: interrupt
}

void Display::Begin_SpritesLocked()
{
	if(m_currentScanline == 143)
	{
		Begin_VBlank();
		return;
	}

	m_currentState = DisplayState::SpritesLocked;
	m_stateTicksRemaining += 80;

	m_currentScanline++;
	if(m_currentScanline >= 144)	///<After a VBlank, this will be 153.  If we've reached this point, then we've started a new frame.
		m_currentScanline = 0;

	//todo: coincident flag
	//todo: interrupt

	//Set mode 10
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_SpriteLock;

	//todo: interrupt
}

void Display::Begin_VideoRamLocked()
{
	m_currentState = DisplayState::VideoRamLocked;
	m_stateTicksRemaining += 172;

	//Set mode 11
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_VideoRamLock;

	//todo: interrupt
}

void Display::Run_VBlank(int ticks)
{
	//Update LY
	m_vblankScanlineTicksRemaining -= ticks;
	if(m_vblankScanlineTicksRemaining <= 0)
	{
		m_currentScanline++;
		m_vblankScanlineTicksRemaining += 456;

		//todo: coincident flag
		//todo: interrupt
	}

	//Swap buffers at the end of VBlank
	if(m_stateTicksRemaining <= ticks)
	{
		EnterCriticalSection( (LPCRITICAL_SECTION)m_screenBufferLock );
			ScreenBuffer* temp = m_stableScreenBuffer;
			m_stableScreenBuffer = m_activeScreenBuffer;
			m_activeScreenBuffer = temp;
		LeaveCriticalSection( (LPCRITICAL_SECTION)m_screenBufferLock );
	}
}

void Display::RenderPixel(int screenX, int screenY)
{
	//Render background
	if(m_lcdControl & LCDC_Background)
	{
		//Which tile map?
		u16 bgTileMapAddress = 0x9800;
		if(m_lcdControl & LCDC_BackgroundTileMap)
			bgTileMapAddress = 0x9c00;

		//Convert screen pixel coordinates to background pixel coordinates
		u8 bgPixelX = (screenX + m_scrollX) % 160;
		u8 bgPixelY = (screenY + m_scrollY) % 144;

		//Convert background pixel coordinates to background tile coordinates
		u8 bgTileX = bgPixelX / 8;
		u8 bgTileY = bgPixelY / 8;
		u16 bgTileIndex = (bgTileY * 32) + bgTileX;	///<BG map is 32x32

		//Get the tile value
		u8 bgTileValue = m_memory->Read8(bgTileMapAddress + bgTileIndex);

		//Which tile data?
		s8 bytesPerTile = 16;
		u16 bgTileAddress = (u16)0x8000 + (bgTileValue * bytesPerTile);
		if( !(m_lcdControl & LCDC_TileData) )
		{
			s8 signedBgTileValue = (s8)bgTileValue;
			bgTileAddress = (u16)0x9000 + (signedBgTileValue * bytesPerTile);
		}
		
		//Adjust the background pixel coordinates to sub-tile coordinates (each tile is 8x8, and we need to know which of those we need)
		u8 bgTilePixelX = bgPixelX % 8;
		u8 bgTilePixelY = bgPixelY % 8;

		//Use the pixel location to find the bytes we need
		u8 bgTileByteOffset = bgTilePixelY * 2;	///<2 bytes per line
		u16 bgTileByteAddress = bgTileAddress + bgTileByteOffset;

		//2 bytes combine to represent a line.  Each byte has 1 bit from each of the 8 pixels in that line.
		u8 bgTileByteLow = m_memory->Read8(bgTileByteAddress);
		u8 bgTileByteHigh = m_memory->Read8(bgTileByteAddress+1);

		//We now have the 2 bytes representing the line.  Now pull out the pixel bit we want from those two bytes.
		u8 bgTilePixelBitOffset = 7 - bgTilePixelX;	///<At x=0, we want bit 7.
		u8 bgPixelValue = (bgTileByteLow & (1<<bgTilePixelBitOffset)) ? 0x01 : 0x00;
		if(bgTileByteHigh & (1<<bgTilePixelBitOffset))
			bgPixelValue |= 0x02;

		//Ok...so we have our pixel.  Now we still have to look it up in the palette.
		u8 bgPixelPaletteShift = bgPixelValue * 2;	///<2 bits per entry
		u8 bgPixelPaletteValue = (m_backgroundPalette & (0x03 << bgPixelPaletteShift)) >> bgPixelPaletteShift;

		//Done
		(*m_activeScreenBuffer)(screenX, screenY).Value = bgPixelPaletteValue;
	}

	//Render sprites

	//?? Render window
}

void Display::RenderScanline()
{
	//todo: LCDC:7 = LCD Controller on/off

	for(int x=0;x<160;x++)
	{
		RenderPixel(x, m_currentScanline);
	}
}