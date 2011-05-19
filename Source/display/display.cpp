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

	m_vramOffset = 0x8000;
	m_oamOffset = 0xfe00;
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

void Display::WriteVram(u16 address, u8 value)
{
	//Update cache
	m_vramCache[address - m_vramOffset] = value;

	//Update tile data
	if(address >= 0x8000 && address < 0x9800)
		UpdateTileData(address, value);
}

void Display::WriteOam(u16 address, u8 value)
{
	m_oamCache[address - m_oamOffset] = value;
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
	CheckCoincidence(); ///<Not sure if this is supposed to be here
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

	//LCDC interrupt
	if(m_lcdStatus & STAT_Interrupt_HBlank)
	{
		u8 interrupts = m_memory->Read8(REG_IF);
		interrupts |= IF_LCDC;
		m_memory->Write8(REG_IF, interrupts);
	}
}

void Display::Begin_VBlank()
{
	m_currentState = DisplayState::VBlank;
	m_stateTicksRemaining += 4560;

	m_currentScanline = 144;
	CheckCoincidence();

	m_vblankScanlineTicksRemaining = m_stateTicksRemaining % 456;

	//Set mode 01
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_VBlank;

	//VBlank interrupt
	u8 interrupts = m_memory->Read8(REG_IF);
	interrupts |= IF_VBLANK;
	m_memory->Write8(REG_IF, interrupts);

	//LCDC interrupt
	if(m_lcdStatus & STAT_Interrupt_VBlank)
	{
		interrupts |= IF_LCDC;
		m_memory->Write8(REG_IF, interrupts);
	}
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

	CheckCoincidence();

	//Set mode 10
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_SpriteLock;

	//LCDC interrupt
	if(m_lcdStatus & STAT_Interrupt_SpriteLock)
	{
		u8 interrupts = m_memory->Read8(REG_IF);
		interrupts |= IF_LCDC;
		m_memory->Write8(REG_IF, interrupts);
	}
}

void Display::Begin_VideoRamLocked()
{
	m_currentState = DisplayState::VideoRamLocked;
	m_stateTicksRemaining += 172;

	//Set mode 11
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_VideoRamLock;

	//No interrupts for this mode
}

void Display::Run_VBlank(int ticks)
{
	//Update LY
	m_vblankScanlineTicksRemaining -= ticks;
	if(m_vblankScanlineTicksRemaining <= 0)
	{
		m_currentScanline++;
		m_vblankScanlineTicksRemaining += 456;

		CheckCoincidence();
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
		u8 bgPixelX = (screenX + m_scrollX) % 256;
		u8 bgPixelY = (screenY + m_scrollY) % 256;

		//Convert background pixel coordinates to background tile coordinates
		u8 bgTileX = bgPixelX / 8;
		u8 bgTileY = bgPixelY / 8;
		u16 bgTileIndex = (bgTileY * 32) + bgTileX;	///<BG map is 32x32

		//Get the tile value
		u8 bgTileValue = m_vramCache[bgTileMapAddress + bgTileIndex - m_vramOffset];

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
		int cacheTileSize = 8*8;
		u8 bgPixelValue = m_tileData[ ((bgTileAddress - 0x8000) * cacheTileSize) + (bgTilePixelY * 8) + bgTilePixelX ];

		//Ok...so we have our pixel.  Now we still have to look it up in the palette.
		u8 bgPixelPaletteShift = bgPixelValue * 2;	///<2 bits per entry
		u8 bgPixelPaletteValue = (m_backgroundPalette & (0x03 << bgPixelPaletteShift)) >> bgPixelPaletteShift;

		//Done
		(*m_activeScreenBuffer)(screenX, screenY).Value = bgPixelPaletteValue;
	}

	//?? Render window

	//Render sprites
	if(m_lcdControl & LCDC_Sprites)
	{
		//Sprites can be 8x8 or 8x16
		u8 spriteWidth = 8;
		u8 spriteHeight = 8;
		if(m_lcdControl & LCDC_SpriteSize)
			spriteHeight = 16;

		u8 spriteTileSize = spriteHeight * 2;

		//Iterate over all sprite entries in the table
		for(int i=0;i<40;i++)
		{
			u16 spriteDataAddress = 0xfe00 + (i*4);	///<4 bytes per sprite entry

			u8 spriteY = m_oamCache[spriteDataAddress - m_oamOffset];
			u8 spriteX = m_oamCache[spriteDataAddress+1 - m_oamOffset];

			//Sprite coordinates are offset, so sprite[8,16] = screen[0,0].
			spriteX -= 8;
			spriteY -= 16;

			//Is the sprite relevant to this pixel?
			if(!(spriteX <= screenX && spriteX+spriteWidth > screenX &&
				spriteY <= screenY && spriteY+spriteHeight > screenY) )
				continue;

			//It's relevant, so get the rest of the data
			u8 spriteTileValue = m_oamCache[spriteDataAddress+2 - m_oamOffset];
			u8 spriteFlags = m_oamCache[spriteDataAddress+3 - m_oamOffset];

			//Is it visible?  (priority vs background and window)
			if(spriteFlags & (1<<7))	///<Lower priority if set, higher priority otherwise
			{
				//Lower priority means the sprite is hidden behind any value except 0
				if( (*m_activeScreenBuffer)(screenX, screenY).Value != 0 )
					continue;
			}

			//Figure out which line we need
			u8 targetTileLine = screenY - spriteY;
			if(spriteFlags & (1<<6))	///<Flip Y if set
				targetTileLine = (spriteHeight-1) - targetTileLine;

			//Figure out where to get the bytes that correspond to this line of the tile
			u16 tileDataAddress = 0x8000 + (spriteTileValue * spriteTileSize);
			u16 tileLineAddress = tileDataAddress + (targetTileLine * 2);

			//Read the two bytes for this line of the tile
			u8 tileLineLow = m_vramCache[tileLineAddress - m_vramOffset];
			u8 tileLineHigh = m_vramCache[tileLineAddress+1 - m_vramOffset];

			//Determine the bit offset for the X value
			u8 bitOffset = screenX - spriteX;
			if(spriteFlags & (1<<5))	///<Flip X if set
				bitOffset = 7 - bitOffset;

			//At bit7, we get the value for x=0.  We need to reverse it to get a shift value.
			bitOffset = 7 - bitOffset;

			//Get the value for the pixel
			u8 pixelValue = (tileLineLow & (1<<bitOffset)) ? 1 : 0;
			if(tileLineHigh & (1<<bitOffset))
				pixelValue |= 0x02;

			//Now look it up in the palette
			u8 pixelPaletteShift = pixelValue * 2;	///<2 bits per palette entry
			u8 pixelPaletteValue = (m_spritePalette0 & (0x03 << pixelPaletteShift)) >> pixelPaletteShift;
			if(spriteFlags & (1<<4))	///<Use sprite palette 1 if set
				pixelPaletteValue = (m_spritePalette1 & (0x03 << pixelPaletteShift)) >> pixelPaletteShift;

			//Done
			(*m_activeScreenBuffer)(screenX, screenY).Value = pixelPaletteValue;
		}
	}
}

void Display::RenderScanline()
{
	//todo: LCDC:7 = LCD Controller on/off

	for(int x=0;x<160;x++)
	{
		RenderPixel(x, m_currentScanline);
	}
}

void Display::CheckCoincidence()
{
	if(m_currentScanline == m_scanlineCompare)
	{
		//LYC=LY flag
		m_lcdStatus |= STAT_Coincidence;

		//LCDC interrupt
		if(m_lcdStatus & STAT_Interrupt_Coincidence)
		{
			u8 interrupts = m_memory->Read8(REG_IF);
			interrupts |= IF_LCDC;
			m_memory->Write8(REG_IF, interrupts);
		}
	}
	else
	{
		//LYC=LY flag
		m_lcdStatus &= ~(STAT_Coincidence);
	}
}

void Display::UpdateTileData(u16 address, u8 data)
{
	int baseVramAddress = address - 0x8000;

	//Get both bytes corresponding to the line
	int lineAddress = baseVramAddress;
	if(baseVramAddress & 1)	///<Bytes 0 and 1 of the line will always start on an even boundary.  If baseVramAddress is odd, then we're modifying the high byte and the low byte is baseVramAddress-1.
		lineAddress--;

	u8 tileDataLow = m_vramCache[lineAddress];
	u8 tileDataHigh = m_vramCache[lineAddress+1];

	//In vram, each tile is 8 lines tall with 2 bytes per line, so each tile is 16 bytes.
	int tileIndex = baseVramAddress / 16;

	//In the local tiledata cache, each tile is 8 lines tall with 8 bytes per line, so each tile is 64 bytes
	int cacheTileAddress = tileIndex * 64;
	int changedLine = (baseVramAddress % 16) / 2;	///<16 bytes per tile, 2 bytes per line
	int cacheLineAddress = cacheTileAddress + (changedLine * 8);	///<Starting address of the line within the cache
	
	//Update the cache
	for(int x=0;x<8;x++)
	{
		m_tileData[cacheLineAddress + x] = (tileDataLow & (1<<(7-x))) ? 1 : 0;
		if(tileDataHigh & (1<<(7-x)))
			m_tileData[cacheLineAddress + x] |= 0x02;
	}
}