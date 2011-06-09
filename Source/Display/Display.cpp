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
#include "windows.h"	///<For critical sections.  We need to lock the screen buffer.

#include "Display.h"

#include "../Common/Machine.h"
#include "../Memory/Memory.h"


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

	for(int y=0;y<144;y++)
	{
		for(int x=0;x<160;x++)
		{
			m_frameBackgroundData.SetPixel(x, y, PIXEL_NOT_CACHED);
			m_frameWindowData.SetPixel(x, y, PIXEL_NOT_CACHED);
			m_frameSpriteData.SetPixel(x, y, PIXEL_NOT_CACHED);
		}
	}

	m_nextPixelToRenderX = 0;
}

Display::~Display()
{
	DeleteCriticalSection((LPCRITICAL_SECTION)m_screenBufferLock);
	delete (LPCRITICAL_SECTION)m_screenBufferLock;
}

void Display::SetMachine(Machine* machine)
{
	m_machine = machine;
	m_memory = machine->GetMemory();

	//Registers
	m_memory->SetRegisterLocation(0x40, &m_lcdControl, false);
	m_memory->SetRegisterLocation(0x41, &m_lcdStatus, false);
	m_memory->SetRegisterLocation(0x42, &m_scrollY, true);
	m_memory->SetRegisterLocation(0x43, &m_scrollX, true);
	m_memory->SetRegisterLocation(0x44, &m_currentScanline, false);
	m_memory->SetRegisterLocation(0x45, &m_scanlineCompare, false);
	m_memory->SetRegisterLocation(0x47, &m_backgroundPalette, true);
	m_memory->SetRegisterLocation(0x48, &m_spritePalette0, true);
	m_memory->SetRegisterLocation(0x49, &m_spritePalette1, true);
	m_memory->SetRegisterLocation(0x4a, &m_windowY, true);
	m_memory->SetRegisterLocation(0x4b, &m_windowX, true);
}

void Display::Initialize()
{
	m_activeScreenBuffer = &m_screenBuffer;
	m_stableScreenBuffer = &m_screenBuffer2;

	SetLcdControl(0x91);
	m_lcdStatus = 0x00;	//???

	m_scrollY = 0;
	m_scrollX = 0;

	m_currentScanline = 0;
	m_scanlineCompare = 0;

	m_backgroundPalette = 0xfc;
	m_spritePalette0 = 0xff;
	m_spritePalette1 = 0xff;

	m_windowX = 0;
	m_windowY = 0;

	//Start everything off at 0,0
	m_currentScanline = 153;
	m_stateTicksRemaining = 0;
	m_vblankScanlineTicksRemaining = 0;
	Begin_SpritesLocked();
}

void Display::Run(int ticks)
{
	if(m_lcdEnabled == false)
		return;

	Render(ticks);

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


void Display::SetLcdControl(u8 value)
{
	m_lcdControl = value;

	if(m_lcdControl & 0x80)
	{
		if(m_lcdEnabled == false)
		{
			//When re-enabling the lcd, it needs to pick up a couple cycles into the sprites-locked phase.
			m_stateTicksRemaining = -4;
			m_currentScanline = -1;	///<Begin_SpritesLocked auto-increments and we want to start at LY=0
			Begin_SpritesLocked();
		}

		m_lcdEnabled = true;
		//CheckCoincidence();	///<Enabling this breaks the prehistorik man title screen
	}
	else
	{
		if(m_currentState == DisplayState::VBlank)
		{
			//Clear caches
			for(int y=0;y<144;y++)
			{
				for(int x=0;x<160;x++)
				{
					m_frameBackgroundData.SetPixel(x, y, PIXEL_NOT_CACHED);
					m_frameWindowData.SetPixel(x, y, PIXEL_NOT_CACHED);
					m_frameSpriteData.SetPixel(x, y, PIXEL_NOT_CACHED);
					m_activeScreenBuffer->SetPixel(x, y, 0);
				}
			}

			//Behaves as though it's in h-blank while disabled
			Begin_HBlank();
			m_currentScanline = 0;
			m_stateTicksRemaining = 0;

			CheckCoincidence();	///<Disabling this breaks bubsy2

			m_lcdEnabled = false;
		}
	}
}

void Display::SetLcdStatus(u8 value)
{
	if(value & STAT_Coincidence)
		m_lcdStatus &= ~(STAT_Coincidence);

	m_lcdStatus &= ~(0x78);
	m_lcdStatus |= (value & 0x78);

	m_lcdStatus |= 0x80;

	CheckCoincidence();	///<Enabling or disabling this seems to have no impact?  Probably because writing STAT_Coincidence can only clear it?
}

void Display::SetCurrentScanline(u8 value)
{
	//todo: something about resetting the counter or stopping the display
	m_currentState = DisplayState::VBlank;
	SetLcdControl(m_lcdControl & 0x7f);
	SetLcdControl(m_lcdControl | 0x80);
	//CheckCoincidence();	///<Enabling or disabling this seems to have no impact?  Why doesn't it break the prehistorik man title?
}

void Display::SetScanlineCompare(u8 value)
{
	m_scanlineCompare = value;
	//CheckCoincidence(); ///<Enabling this causes prehistorik man to hang sooner (beginning of "START" instead of the end)
}

void Display::Begin_HBlank()
{
	m_currentState = DisplayState::HBlank;
	m_stateTicksRemaining += 204;

	//Set mode 00
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_HBlank;

	//Unlock vram and oam
	m_memory->SetVramLock(false);
	m_memory->SetOamLock(false);

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
	CheckCoincidence();	///<Disabling this has no impact?  IF_VBLANK is higher priority... but some games might still rely on LCDC LYC=LY only?

	m_vblankScanlineTicksRemaining = m_stateTicksRemaining % 456;
	if(m_vblankScanlineTicksRemaining == 0)
		m_vblankScanlineTicksRemaining += 456;

	//Set mode 01
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_VBlank;

	//Unlock vram and oam
	m_memory->SetVramLock(false);
	m_memory->SetOamLock(false);
	
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

	CheckCoincidence();	///<This one's important.  Disabling it breaks lots of stuff.

	//Set mode 10
	m_lcdStatus &= ~(STAT_Mode);
	m_lcdStatus |= Mode_SpriteLock;

	//Lock oam only
	m_memory->SetVramLock(false);
	m_memory->SetOamLock(true);

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

	//Lock vram and oam
	m_memory->SetVramLock(true);
	m_memory->SetOamLock(true);

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

		CheckCoincidence();	///<This one's important.  Disabling it breaks things.
	}

	//End of VBlank
	if(m_stateTicksRemaining <= ticks)
	{
		//Swap buffers
		EnterCriticalSection( (LPCRITICAL_SECTION)m_screenBufferLock );
			ScreenBuffer* temp = m_stableScreenBuffer;
			m_stableScreenBuffer = m_activeScreenBuffer;
			m_activeScreenBuffer = temp;
		LeaveCriticalSection( (LPCRITICAL_SECTION)m_screenBufferLock );

		//Clear caches
		for(int y=0;y<144;y++)
		{
			for(int x=0;x<160;x++)
			{
				m_frameBackgroundData.SetPixel(x, y, PIXEL_NOT_CACHED);
				m_frameWindowData.SetPixel(x, y, PIXEL_NOT_CACHED);
				m_frameSpriteData.SetPixel(x, y, PIXEL_NOT_CACHED);
				m_activeScreenBuffer->SetPixel(x, y, 0);
			}
		}
	}
}

void Display::RenderBackgroundPixel(int screenX, int screenY)
{
	if((m_lcdControl & LCDC_Background) == 0)
		return;

	//Cached?
	u8 cachedValue = m_frameBackgroundData.GetPixel(screenX, screenY);
	if(cachedValue != PIXEL_NOT_CACHED)
	{
		m_activeScreenBuffer->SetPixel(screenX, screenY, cachedValue);
		return;
	}

	//Which tile map?
	u16 bgTileMapAddress = 0x9800;
	if(m_lcdControl & LCDC_BackgroundTileMap)
		bgTileMapAddress = 0x9c00;

	//Convert screen pixel coordinates to background pixel coordinates
	u8 bgPixelX = (u8)(screenX + m_scrollX);
	u8 bgPixelY = (u8)(screenY + m_scrollY);

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

	//Get the tile data index
	int tileDataIndex = (bgTileAddress - 0x8000) / 16;

	//Find the tile in the tile data cache
	int cacheTileAddress = tileDataIndex * 64;

	//Get the pixel we need
	int tilePixelX = bgPixelX % 8;
	int tilePixelY = bgPixelY % 8;

	//Write all the tile pixels on this line to the cache
	int cacheScreenX = screenX;
	while(tilePixelX <= 7)
	{
		u8 bgPixelValue = m_tileData[ cacheTileAddress + (tilePixelY * 8) + tilePixelX ];

		//Ok...so we have our pixel.  Now we still have to look it up in the palette.
		int bgPixelPaletteShift = bgPixelValue * 2;	///<2 bits per entry
		u8 bgPixelPaletteValue = (m_backgroundPalette & (0x03 << bgPixelPaletteShift)) >> bgPixelPaletteShift;

		//Done
		m_frameBackgroundData.SetPixel(cacheScreenX, screenY, bgPixelPaletteValue);

		tilePixelX++;
		cacheScreenX++;
	}

	//Write this pixel out of the cache
	cachedValue = m_frameBackgroundData.GetPixel(screenX, screenY);
	if(cachedValue != PIXEL_NOT_CACHED)
		m_activeScreenBuffer->SetPixel(screenX, screenY, cachedValue);
}

void Display::RenderSpritePixel(int screenX, int screenY)
{
	if((m_lcdControl & LCDC_Sprites) == 0)
		return;

	//Check priority
	if(m_spriteHasPriority[screenX] == false && m_activeScreenBuffer->GetPixel(screenX, screenY) != 0)
		return;

	//RenderSprites fills m_frameSpriteData.  If there's no value there at this pixel, then there's no sprite at this pixel.
	u8 cachedValue = m_frameSpriteData.GetPixel(screenX, screenY);
	if(cachedValue != PIXEL_NOT_CACHED && cachedValue != PIXEL_TRANSPARENT)
		m_activeScreenBuffer->SetPixel(screenX, screenY, cachedValue);
}

void Display::RenderWindowPixel(int screenX, int screenY)
{
	if((m_lcdControl & LCDC_Window) == 0)
		return;

	//Visible?
	if(screenX + 7 < m_windowX || screenY < m_windowY)
		return;
	
	//Cached?
	u8 cachedValue = m_frameWindowData.GetPixel(screenX, screenY);
	if(cachedValue != PIXEL_NOT_CACHED)
	{
		m_activeScreenBuffer->SetPixel(screenX, screenY, cachedValue);
		return;
	}

	//Which tile map?
	u16 tileMapAddress = 0x9800;
	if(m_lcdControl & LCDC_WindowTileMap)
		tileMapAddress = 0x9c00;

	//Convert screen pixel coordinates to window pixel coordinates
	u8 windowPixelX = screenX - (m_windowX - 7);
	u8 windowPixelY = screenY - m_windowY;

	//Convert window pixel coordinates to tile coordinates
	u8 tileX = windowPixelX / 8;
	u8 tileY = windowPixelY / 8;
	u16 tilePositionIndex = (tileY * 32) + tileX;	///<BG map is 32x32

	//Get the tile value
	u8 tileValue = m_vramCache[tileMapAddress + tilePositionIndex - m_vramOffset];

	//Which tile data?
	s8 bytesPerTile = 16;
	u16 tileAddress = (u16)0x8000 + (tileValue * bytesPerTile);
	if( !(m_lcdControl & LCDC_TileData) )
	{
		s8 signedTileValue = (s8)tileValue;
		tileAddress = (u16)0x9000 + (signedTileValue * bytesPerTile);
	}

	//Get the tile index
	int tileIndex = (tileAddress - 0x8000) / 16;

	//Find the tile in the tile data cache
	int cacheTileAddress = tileIndex * 64;

	//Get the pixel we need
	int tilePixelX = windowPixelX % 8;
	int tilePixelY = windowPixelY % 8;

	//Write all the tile pixels on this line to the cache
	int cacheScreenX = screenX;
	while(tilePixelX <= 7)
	{
		u8 pixelValue = m_tileData[ cacheTileAddress + (tilePixelY * 8) + tilePixelX ];

		//Ok...so we have our pixel.  Now we still have to look it up in the palette.
		int pixelPaletteShift = pixelValue * 2;	///<2 bits per entry
		u8 pixelPaletteValue = (m_backgroundPalette & (0x03 << pixelPaletteShift)) >> pixelPaletteShift;

		//Done
		m_frameWindowData.SetPixel(cacheScreenX, screenY, pixelPaletteValue);

		tilePixelX++;
		cacheScreenX++;
	}

	//Write this pixel out of the cache
	cachedValue = m_frameWindowData.GetPixel(screenX, screenY);
	if(cachedValue != PIXEL_NOT_CACHED)
		m_activeScreenBuffer->SetPixel(screenX, screenY, cachedValue);
}

void Display::RenderSprites(int screenY)
{
	//Default to no priority
	for(int i=0;i<160;i++)
		m_spriteHasPriority[i] = false;

	//Sprites can be 8x8 or 8x16
	int spriteWidth = 8;
	int spriteHeight = 8;
	if(m_lcdControl & LCDC_SpriteSize)
		spriteHeight = 16;

	int spriteTileSize = 16;

	//Iterate over all sprite entries in the table
	for(int i=0;i<40;i++)
	{
		u16 spriteDataAddress = 0xfe00 + (i*4);

		int spriteY = m_oamCache[spriteDataAddress - m_oamOffset];
		int spriteX = m_oamCache[spriteDataAddress+1 - m_oamOffset];

		//Sprite coordinates are offset, so sprite[8,16] = screen[0,0].
		spriteX -= 8;
		spriteY -= 16;

		//Is the sprite relevant to this line?
		if(spriteY > screenY || spriteY+spriteHeight <= screenY)
			continue;

		//Is the sprite visible?
		if(spriteX+spriteWidth < 0 || spriteX >= 160)
			continue;

		//It's relevant, so get the rest of the data
		int spriteTileValue = m_oamCache[spriteDataAddress+2 - m_oamOffset];
		int spriteFlags = m_oamCache[spriteDataAddress+3 - m_oamOffset];

		//In 8x16 mode, the least significant bit of the tile value is ignored
		if(m_lcdControl & LCDC_SpriteSize)
			spriteTileValue &= ~(0x01);

		//Figure out which line we need
		int targetTileLine = screenY - spriteY;
		if(spriteFlags & (1<<6))	///<Flip Y if set
			targetTileLine = (spriteHeight-1) - targetTileLine;

		//Figure out where to get the bytes that correspond to this line of the tile
		u16 tileDataAddress = 0x8000 + (spriteTileValue * spriteTileSize);
		u16 tileLineAddress = tileDataAddress + (targetTileLine * 2);

		//Read the two bytes for this line of the tile
		u8 tileLineLow = m_vramCache[tileLineAddress - m_vramOffset];
		u8 tileLineHigh = m_vramCache[tileLineAddress+1 - m_vramOffset];

		//Cache the rest of the sprite values on this line
		int tileX = 0;
		int cacheScreenX = spriteX;

		int tileY = screenY - spriteY;
		int cacheScreenY = screenY;

		while(tileX <= 7)
		{
			//Have we already drawn a sprite at this pixel?
			u8 cachedValue = m_frameSpriteData.GetPixel(cacheScreenX, cacheScreenY);
			if(cachedValue != PIXEL_NOT_CACHED && cachedValue != PIXEL_TRANSPARENT)
			{
				tileX++;
				cacheScreenX++;
				continue;
			}

			//Determine the bit offset for the X value
			int bitOffset = tileX;
			if(spriteFlags & (1<<5))	///<Flip X if set
				bitOffset = 7 - bitOffset;

			//At bit7, we get the value for x=0.  We need to reverse it to get a shift value.
			bitOffset = 7 - bitOffset;

			//Get the value for the pixel
			u8 pixelValue = (tileLineLow & (1<<bitOffset)) ? 1 : 0;
			if(tileLineHigh & (1<<bitOffset))
				pixelValue |= 0x02;

			//Transparent?
			if(pixelValue == 0)
			{
				m_frameSpriteData.SetPixel(cacheScreenX, cacheScreenY, PIXEL_TRANSPARENT);
			}
			else
			{
				//Look it up in the palette
				u8 pixelPaletteShift = pixelValue * 2;	///<2 bits per palette entry
				u8 pixelPaletteValue = (m_spritePalette0 & (0x03 << pixelPaletteShift)) >> pixelPaletteShift;
				if(spriteFlags & (1<<4))	///<Use sprite palette 1 if set
					pixelPaletteValue = (m_spritePalette1 & (0x03 << pixelPaletteShift)) >> pixelPaletteShift;

				//Write the pixel
				m_frameSpriteData.SetPixel(cacheScreenX, cacheScreenY, pixelPaletteValue);

				//Save the priority
				if(spriteFlags & (1<<7))	///<Lower priority if set, higher priority otherwise
					m_spriteHasPriority[cacheScreenX] = false;
				else
					m_spriteHasPriority[cacheScreenX] = true;
			}

			cacheScreenX++;
			tileX++;
		}
	}
}

void Display::Render(int ticks)
{
	//Finish rendering the line if necessary
	if(m_currentState == DisplayState::HBlank && m_ticksSpentThisScanline != 0)
	{
		for(; m_nextPixelToRenderX < 160; m_nextPixelToRenderX++)
		{
			RenderBackgroundPixel(m_nextPixelToRenderX, m_currentScanline);
			RenderWindowPixel(m_nextPixelToRenderX, m_currentScanline);
			RenderSpritePixel(m_nextPixelToRenderX, m_currentScanline);
		}

		m_ticksSpentThisScanline = 0;
		m_nextPixelToRenderX = 0;
	}

	//Rendering only occurs during mode 11
	if(m_currentState != DisplayState::VideoRamLocked)
		return;

	if(m_stateTicksRemaining > 160)
		return;

	//Figure out how many pixels we should have rendered by now
	static const float TicksPerPixel = 1.f;	///< Render during 160 of the mode 11 ticks (172).
	m_ticksSpentThisScanline += ticks;
	int numPixelsRendered = (int)( (float)m_ticksSpentThisScanline / TicksPerPixel );
	if(numPixelsRendered > 160)
		numPixelsRendered = 160;

	//Cache the sprites if they haven't been cached yet
	if(m_nextPixelToRenderX == 0)
		RenderSprites(m_currentScanline);

	//Render them
	for(; m_nextPixelToRenderX < numPixelsRendered; m_nextPixelToRenderX++)
	{
		RenderBackgroundPixel(m_nextPixelToRenderX, m_currentScanline);
		RenderWindowPixel(m_nextPixelToRenderX, m_currentScanline);
		RenderSpritePixel(m_nextPixelToRenderX, m_currentScanline);
	}
}

void Display::CheckCoincidence()
{
	if(m_lcdEnabled == false)
		return;

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
	int changedLine = (lineAddress % 16) / 2;	///<16 bytes per tile, 2 bytes per line
	int cacheLineAddress = cacheTileAddress + (changedLine * 8);	///<Starting address of the line within the cache
	
	//Update the cache
	for(int x=0;x<8;x++)
	{
		m_tileData[cacheLineAddress + x] = (tileDataLow & (1<<(7-x))) ? 1 : 0;
		if(tileDataHigh & (1<<(7-x)))
			m_tileData[cacheLineAddress + x] |= 0x02;
	}
}