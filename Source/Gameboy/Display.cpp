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
#include "Display.h"
using namespace Emunisce;

#include "GameboyIncludes.h"

#include "Serialization/SerializationIncludes.h"


//70224 t-states per frame (59.7fps)
//4560 t-states per v-blank (mode 01)

//80 (77-83) t-states per line in mode 10 (oam in use)
//172 (169-175) t-states per line in mode 11 (oam + vram in use)
//204 (201-207) t-states per line in mode 00 (h-blank)


Display::Display()
{
	m_activeScreenBuffer = &m_screenBuffer;
	m_stableScreenBuffer = &m_screenBuffer2;

	m_screenBufferCount = 0;

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

	m_displayPalette[0] = DisplayPixelFromRGBA(1.00f, 1.00f, 1.00f);
	m_displayPalette[1] = DisplayPixelFromRGBA(0.67f, 0.67f, 0.67f);
	m_displayPalette[2] = DisplayPixelFromRGBA(0.33f, 0.33f, 0.33f);
	m_displayPalette[3] = DisplayPixelFromRGBA(0.00f, 0.00f, 0.00f);

	for(int i=0;i<8*4;i++)	///<8 palettes * 4 colors per palette
	{
		m_cgbBackgroundDisplayColor[i] = DisplayPixelFromRGBA(1.f, 1.f, 1.f);
		m_cgbSpriteDisplayColor[i] = DisplayPixelFromRGBA(1.f, 1.f, 1.f);
	}
}

Display::~Display()
{
}


ScreenResolution Display::GetScreenResolution()
{
	ScreenResolution resolution;
	resolution.width = 160;
	resolution.height = 144;

	return resolution;
}

ScreenBuffer* Display::GetStableScreenBuffer()
{
    if(m_screenBufferCopyId == m_screenBufferCount)
        return &m_screenBufferCopy;

    m_screenBufferLock.Acquire();
        m_screenBufferCopy = *m_stableScreenBuffer;
        m_screenBufferCopyId = m_screenBufferCount;
    m_screenBufferLock.Release();

	return &m_screenBufferCopy;
}

int Display::GetScreenBufferCount()
{
	return m_screenBufferCount;
}


void Display::SetMachine(Gameboy* machine)
{
	m_machine = machine;
	m_machineType = machine->GetType();

	m_memory = machine->GetGbMemory();

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
	
	m_memory->SetRegisterLocation(0x68, &m_cgbBackgroundPaletteIndex, false);
	m_memory->SetRegisterLocation(0x69, &m_cgbBackgroundPaletteData, false);
	m_memory->SetRegisterLocation(0x6a, &m_cgbSpritePaletteIndex, false);
	m_memory->SetRegisterLocation(0x6b, &m_cgbSpritePaletteData, false);
}

void Display::Initialize()
{
	m_activeScreenBuffer = &m_screenBuffer;
	m_stableScreenBuffer = &m_screenBuffer2;

	SetLcdControl(0x00);	///<Enabled explicitly by the bios, but it's off before that
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

	m_cgbBackgroundPaletteIndex = 0;
	m_cgbSpritePaletteIndex = 0;

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
		{
			m_memory->EndHBlank();
			Begin_SpritesLocked();	///<This will trigger VBlank when appropriate
		}
	}
}

void Display::Serialize(Archive& archive)
{
	SerializeItem(archive, m_currentState);
	SerializeItem(archive, m_stateTicksRemaining);
	SerializeItem(archive, m_vblankScanlineTicksRemaining);

	SerializeItem(archive, m_screenBufferCount);

	SerializeItem(archive, m_nextPixelToRenderX);
	SerializeItem(archive, m_ticksSpentThisScanline);

	for(int i=0;i<160;i++)
		SerializeItem(archive, m_spriteHasPriority[i]);


	// Screens  (these use a ton of space, but the first frame is inaccurate without them)

	//m_screenBuffer.Serialize(archive);
	//m_screenBuffer2.Serialize(archive);
	//m_frameBackgroundData.Serialize(archive);
	//m_frameWindowData.Serialize(archive);
	//m_frameSpriteData.Serialize(archive);



	// Caches

	SerializeItem(archive, m_vramOffset);
	SerializeItem(archive, m_oamOffset);


	// Registers

	SerializeItem(archive, m_lcdControl);
	SerializeItem(archive, m_lcdStatus);

	SerializeItem(archive, m_scrollY);
	SerializeItem(archive, m_scrollX);

	SerializeItem(archive, m_currentScanline);
	SerializeItem(archive, m_scanlineCompare);

	SerializeItem(archive, m_backgroundPalette);
	SerializeItem(archive, m_spritePalette0);
	SerializeItem(archive, m_spritePalette1);

	SerializeItem(archive, m_windowX);
	SerializeItem(archive, m_windowY);


	// Properties from registers

	SerializeItem(archive, m_lcdEnabled);


	// CGB

	SerializeItem(archive, m_cgbBackgroundPaletteIndex);
	SerializeItem(archive, m_cgbBackgroundPaletteData);

	SerializeItem(archive, m_cgbSpritePaletteIndex);
	SerializeItem(archive, m_cgbSpritePaletteData);

	for(int i=0;i<8*4;i++)	///<8 palettes, 4 colors per palette
	{
		SerializeItem(archive, m_cgbBackgroundPaletteColor[i]);
		SerializeItem(archive, m_cgbBackgroundDisplayColor[i]);

		SerializeItem(archive, m_cgbSpritePaletteColor[i]);
		SerializeItem(archive, m_cgbSpriteDisplayColor[i]);
	}
}

void Display::WriteVram(int bank, u16 address, u8 value)
{
	//This shouldn't be here since Memory handles the actual write, but it has to 
	// be here for now because UpdateTileData assumes the value has already been written.
	u8* vram = m_memory->GetVram(bank);
	vram[address - 0x8000] = value;

	//Update tile data
	if(address >= 0x8000 && address < 0x9800)
		UpdateTileData(bank, address, value);
}

void Display::WriteOam(u16 address, u8 value)
{
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
					m_activeScreenBuffer->SetPixel(x, y, m_displayPalette[0]);
				}
			}

			//Behaves as though it's in h-blank while disabled
			Begin_HBlank();
			m_memory->EndHBlank();	///<No HBlank DMA if the screen is just disabled
			m_currentScanline = 0;
			m_stateTicksRemaining = 0;

			CheckCoincidence();	///<Disabling this breaks bubsy2

			m_lcdEnabled = false;
		}
	}
}

void Display::SetLcdStatus(u8 value)
{
	//Writing to bit 3 clears it in this register
	if(value & STAT_Coincidence)
		m_lcdStatus &= ~(STAT_Coincidence);

	m_lcdStatus &= ~(0x78);
	m_lcdStatus |= (value & 0x78);

	m_lcdStatus |= 0x80;

	CheckCoincidence();	///<Enabling or disabling this seems to have no impact?  Probably because writing STAT_Coincidence can only clear it?

	//"STAT bug" causes an interrupt to fire if this register is written to during h-blank or v-blank
	if(m_lcdEnabled == true)
	{
		if(m_currentState == DisplayState::HBlank || m_currentState == DisplayState::VBlank)
		{
			u8 interrupts = m_memory->Read8(REG_IF);
			interrupts |= IF_LCDC;
			m_memory->Write8(REG_IF, interrupts);
		}
	}
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
	//CheckCoincidence(); ///<Enabling this causes prehistorik man to hang
}


void Display::SetCgbBackgroundPaletteTarget(u8 value)
{
	m_cgbBackgroundPaletteIndex = value;

	int index = value & 0x3f;
	index /= 2;
	u16 paletteData = m_cgbBackgroundPaletteColor[index];

	if(value & 0x01)
		m_cgbBackgroundPaletteData = (u8)(paletteData >> 8);
	else
		m_cgbBackgroundPaletteData = (u8)paletteData;
}

void Display::SetCgbBackgroundPaletteData(u8 value)
{
	static const float cgbToRgb = (float)0xff / (float)0x1f;

	int index = m_cgbBackgroundPaletteIndex;

	bool autoIncrement = (index & 0x80) ? true : false;
	index &= 0x3f;

	bool highByte = (index & 0x01) ? true : false;
	index /= 2;

	u16 paletteData = m_cgbBackgroundPaletteColor[index];

	if(highByte)
	{
		paletteData &= 0x00ff;
		paletteData |= (value << 8);
	}
	else
	{
		paletteData &= 0xff00;
		paletteData |= value;
	}

	m_cgbBackgroundPaletteColor[index] = paletteData;
	m_cgbBackgroundPaletteData = value;

	u8 r = (u8)((float)((paletteData & (0x1f << 0)) >> 0) * cgbToRgb);		///<0b0000000000011111
	u8 g = (u8)((float)((paletteData & (0x1f << 5)) >> 5) * cgbToRgb);		///<0b0000001111100000
	u8 b = (u8)((float)((paletteData & (0x1f << 10)) >> 10) * cgbToRgb);	///<0b0111110000000000
	m_cgbBackgroundDisplayColor[index] = DisplayPixelFromRGBA(r, g, b);

	if(autoIncrement)
	{
		m_cgbBackgroundPaletteIndex++;
		m_cgbBackgroundPaletteIndex &= ~(0x40);	///<clears bit 6 (which is unusable) so we don't overflow

		SetCgbBackgroundPaletteTarget(m_cgbBackgroundPaletteIndex); ///<Update m_cgbBackgroundPaletteData
	}
}


void Display::SetCgbSpritePaletteTarget(u8 value)
{
	m_cgbSpritePaletteIndex = value;

	int index = value & 0x3f;
	index /= 2;
	u16 paletteData = m_cgbSpritePaletteColor[index];

	if(value & 0x01)
		m_cgbSpritePaletteData = (u8)(paletteData >> 8);
	else
		m_cgbSpritePaletteData = (u8)paletteData;
}

void Display::SetCgbSpritePaletteData(u8 value)
{
	static const float cgbToRgb = (float)0xff / (float)0x1f;

	int index = m_cgbSpritePaletteIndex;

	bool autoIncrement = (index & 0x80) ? true : false;
	index &= 0x3f;

	bool highByte = (index & 0x01) ? true : false;
	index /= 2;

	u16 paletteData = m_cgbSpritePaletteColor[index];

	if(highByte)
	{
		paletteData &= 0x00ff;
		paletteData |= (value << 8);
	}
	else
	{
		paletteData &= 0xff00;
		paletteData |= value;
	}

	m_cgbSpritePaletteColor[index] = paletteData;
	m_cgbSpritePaletteData = value;

	u8 r = (u8)((float)((paletteData & (0x1f << 0)) >> 0) * cgbToRgb);		///<0b0000000000011111
	u8 g = (u8)((float)((paletteData & (0x1f << 5)) >> 5) * cgbToRgb);		///<0b0000001111100000
	u8 b = (u8)((float)((paletteData & (0x1f << 10)) >> 10) * cgbToRgb);	///<0b0111110000000000
	m_cgbSpriteDisplayColor[index] = DisplayPixelFromRGBA(r, g, b);

	if(autoIncrement)
	{
		m_cgbSpritePaletteIndex++;
		m_cgbSpritePaletteIndex &= ~(0x40);	///<clears bit 6 (which is unusable) so we don't overflow
		
		SetCgbSpritePaletteTarget(m_cgbSpritePaletteIndex); ///<Update m_cgbSpritePaletteData
	}
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

	//Allow HBlank DMA
	m_memory->BeginHBlank();

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
		m_screenBufferLock.Acquire();
            GameboyScreenBuffer* temp = m_stableScreenBuffer;
            m_stableScreenBuffer = m_activeScreenBuffer;
            m_activeScreenBuffer = temp;

            m_screenBufferCount++;
		m_screenBufferLock.Release();

		//Clear caches
		for(int y=0;y<144;y++)
		{
			for(int x=0;x<160;x++)
			{
				m_frameBackgroundData.SetPixel(x, y, PIXEL_NOT_CACHED);
				m_frameWindowData.SetPixel(x, y, PIXEL_NOT_CACHED);
				m_frameSpriteData.SetPixel(x, y, PIXEL_NOT_CACHED);
				m_activeScreenBuffer->SetPixel(x, y, m_displayPalette[0]);
			}
		}
	}
}

void Display::RenderBackgroundPixel(int screenX, int screenY)
{
	if((m_lcdControl & LCDC_Background) == 0 && m_machineType != EmulatedMachine::GameboyColor)
		return;

	//Cached?
	DisplayPixel cachedValue = m_frameBackgroundData.GetPixel(screenX, screenY);
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
	u8* vram = m_memory->GetVram(0);
	u8 bgTileValue = vram[bgTileMapAddress + bgTileIndex - m_vramOffset];

	//Get the tile attributes (cgb only)
	u8* cgbVram = NULL;
	u8 bgTileAttributes = 0;
	if(m_machineType == EmulatedMachine::GameboyColor)
	{
		cgbVram = m_memory->GetVram(1);
		bgTileAttributes = cgbVram[bgTileMapAddress + bgTileIndex - m_vramOffset];
	}

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
		int bank = 0;
		if(m_machineType == EmulatedMachine::GameboyColor)
		{
			if(bgTileAttributes & 0x08)
				bank = 1;
		}

		u8 bgPixelValue = m_tileData[bank][ cacheTileAddress + (tilePixelY * 8) + tilePixelX ];
		DisplayPixel finalValue = m_displayPalette[0];	///<Re-assigned after a palette lookup

		//Ok...so we have our pixel.  Now we still have to look it up in the palette.
		if(m_machineType == EmulatedMachine::GameboyColor)
		{
			//CGB uses one of 8 background color palette registers
			int paletteIndex = bgTileAttributes & 0x07;
			finalValue = m_cgbBackgroundDisplayColor[(paletteIndex * 4) + bgPixelValue];	///<4 colors per palette
		}
		else
		{
			//DMG uses the background palette register
			int bgPixelPaletteShift = bgPixelValue * 2;	///<2 bits per entry
			u8 bgPixelPaletteValue = (m_backgroundPalette & (0x03 << bgPixelPaletteShift)) >> bgPixelPaletteShift;
			finalValue = m_displayPalette[ bgPixelPaletteValue ];
		}

		//Done
		m_frameBackgroundData.SetPixel(cacheScreenX, screenY, finalValue);

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
	if(m_machineType != EmulatedMachine::GameboyColor)
	{
		if(m_spriteHasPriority[screenX] == false && m_activeScreenBuffer->GetPixel(screenX, screenY) != m_displayPalette[0])
			return;
	}
	else /*GBC*/
	{
		if(m_lcdControl & LCDC_Background)	///<LCDC_Background in CGB mode is a master priority flag.  If 0, then sprites have priority.
		{
			//todo: background priority flag, background data != 00 (palettes)
		}
	}

	//RenderSprites fills m_frameSpriteData.  If there's no value there at this pixel, then there's no sprite at this pixel.
	DisplayPixel cachedValue = m_frameSpriteData.GetPixel(screenX, screenY);
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
	DisplayPixel cachedValue = m_frameWindowData.GetPixel(screenX, screenY);
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
	u8* vram = m_memory->GetVram(0);
	u8 tileValue = vram[tileMapAddress + tilePositionIndex - m_vramOffset];

	//Get the tile attributes (cgb only)
	u8* cgbVram = NULL;
	u8 tileAttributes = 0;
	if(m_machineType == EmulatedMachine::GameboyColor)
	{
		cgbVram = m_memory->GetVram(1);
		tileAttributes = cgbVram[tileMapAddress + tilePositionIndex - m_vramOffset];
	}

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
		int bank = 0;
		if(m_machineType == EmulatedMachine::GameboyColor)
		{
			if(tileAttributes & 0x08)
				bank = 1;
		}

		u8 pixelValue = m_tileData[bank][ cacheTileAddress + (tilePixelY * 8) + tilePixelX ];
		DisplayPixel finalValue = m_displayPalette[0];	///<value is overwritten after the palette lookup

		//Ok...so we have our pixel.  Now we still have to look it up in the palette.
		if(m_machineType == EmulatedMachine::GameboyColor)
		{
			//CGB uses one of 8 background color palette registers
			int paletteIndex = tileAttributes & 0x07;
			finalValue = m_cgbBackgroundDisplayColor[(paletteIndex * 4) + pixelValue];	///<4 colors per palette
		}
		else
		{
			//DMG uses the background palette register
			int PixelPaletteShift = pixelValue * 2;	///<2 bits per entry
			u8 PixelPaletteValue = (m_backgroundPalette & (0x03 << PixelPaletteShift)) >> PixelPaletteShift;
			finalValue = m_displayPalette[ PixelPaletteValue ];
		}

		//Done
		m_frameWindowData.SetPixel(cacheScreenX, screenY, finalValue);

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

		u8* oam = m_memory->GetOam();
		int spriteY = oam[spriteDataAddress - m_oamOffset];
		int spriteX = oam[spriteDataAddress+1 - m_oamOffset];

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
		int spriteTileValue = oam[spriteDataAddress+2 - m_oamOffset];
		int spriteFlags = oam[spriteDataAddress+3 - m_oamOffset];

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

		//Figure out which bank to read the tile data from (cgb only)
		int bank = 0;
		if(m_machineType == EmulatedMachine::GameboyColor && (spriteFlags & 0x08))
			bank = 1;

		//Read the two bytes for this line of the tile
		u8* vram = m_memory->GetVram(bank);
		u8 tileLineLow = vram[tileLineAddress - m_vramOffset];
		u8 tileLineHigh = vram[tileLineAddress+1 - m_vramOffset];

		//Cache the rest of the sprite values on this line
		int tileX = 0;
		int cacheScreenX = spriteX;

		int cacheScreenY = screenY; ///<todo: This is leftover from when full tiles were cached (as opposed to only the current line).  It's safe to remove this.

		while(tileX <= 7)
		{
			//Have we already drawn a sprite at this pixel?
			DisplayPixel cachedValue = m_frameSpriteData.GetPixel(cacheScreenX, cacheScreenY);
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
				DisplayPixel finalValue = m_displayPalette[0];	///<value is overwritten after the palette lookup

				//Ok...so we have our pixel.  Now we still have to look it up in the palette.
				if(m_machineType == EmulatedMachine::GameboyColor)
				{
					//CGB uses one of 8 background color palette registers
					int paletteIndex = spriteFlags & 0x07;
					finalValue = m_cgbSpriteDisplayColor[(paletteIndex * 4) + pixelValue];	///<4 colors per palette
				}
				else
				{
					//DMG uses one of 2 sprite palette registers
					u8 pixelPaletteShift = pixelValue * 2;	///<2 bits per palette entry
					u8 pixelPaletteValue = (m_spritePalette0 & (0x03 << pixelPaletteShift)) >> pixelPaletteShift;
					if(spriteFlags & (1<<4))	///<Use sprite palette 1 if set
						pixelPaletteValue = (m_spritePalette1 & (0x03 << pixelPaletteShift)) >> pixelPaletteShift;

					finalValue = m_displayPalette[ pixelPaletteValue ];
				}

				//Write the pixel
				m_frameSpriteData.SetPixel(cacheScreenX, cacheScreenY, finalValue);

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

void Display::UpdateTileData(int bank, u16 address, u8 data)
{
	if(bank < 0 || bank > 1)
		return;

	u8* vram = m_memory->GetVram(bank);
	int baseVramAddress = address - 0x8000;

	//Get both bytes corresponding to the line
	int lineAddress = baseVramAddress;
	if(baseVramAddress & 1)	///<Bytes 0 and 1 of the line will always start on an even boundary.  If baseVramAddress is odd, then we're modifying the high byte and the low byte is baseVramAddress-1.
		lineAddress--;

	u8 tileDataLow = vram[lineAddress];
	u8 tileDataHigh = vram[lineAddress+1];

	//In vram, each tile is 8 lines tall with 2 bytes per line, so each tile is 16 bytes.
	int tileIndex = baseVramAddress / 16;

	//In the local tiledata cache, each tile is 8 lines tall with 8 bytes per line, so each tile is 64 bytes
	int cacheTileAddress = tileIndex * 64;
	int changedLine = (lineAddress % 16) / 2;	///<16 bytes per tile, 2 bytes per line
	int cacheLineAddress = cacheTileAddress + (changedLine * 8);	///<Starting address of the line within the cache

	//Update the cache
	for(int x=0;x<8;x++)
	{
		u8 pixelValue = 0;

		if(tileDataLow & (1<<(7-x)))
			pixelValue |= 0x01;

		if(tileDataHigh & (1<<(7-x)))
			pixelValue |= 0x02;

		m_tileData[bank][cacheLineAddress + x] = pixelValue;
	}
}
