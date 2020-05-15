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
#ifndef DISPLAY_H
#define DISPLAY_H

#include <mutex>

#include "PlatformIncludes.h"

#include "MachineIncludes.h"
#include "GameboyTypes.h"

#include "stdlib.h"

#define PIXEL_NOT_CACHED (DisplayPixelFromRGBA((u8)254, (u8)0, (u8)254, (u8)255))
#define PIXEL_TRANSPARENT (DisplayPixelFromRGBA((u8)255, (u8)0, (u8)255, (u8)255))


namespace Emunisce
{

typedef TScreenBuffer<160, 144> GameboyScreenBuffer;

class Display : public IEmulatedDisplay
{
public:

	Display();
	virtual ~Display();


	// IEmulatedDisplay

	ScreenResolution GetScreenResolution() override;
	ScreenBuffer* GetStableScreenBuffer() override;
	int GetScreenBufferCount() override;	///<Returns the id of the current screen buffer.  Not guaranteed to be unique or sequential, so use != when polling for changes.


	// Display

	//Component
	void SetMachine(Gameboy* machine);
	void Initialize();

	void Run(int ticks);

	virtual void Serialize(Archive& archive);


	//Notifications
	void WriteVram(int bank, u16 address, u8 value);
	void WriteOam(u16 address, u8 value);


	//Gameboy registers

	void SetLcdControl(u8 value);
	void SetLcdStatus(u8 value);
	void SetCurrentScanline(u8 value);
	void SetScanlineCompare(u8 value);

	void SetCgbBackgroundPaletteTarget(u8 value);
	void SetCgbBackgroundPaletteData(u8 value);

	void SetCgbSpritePaletteTarget(u8 value);
	void SetCgbSpritePaletteData(u8 value);


private:

	struct DisplayState	///<Can't declare a namespace inside a class, so I'm cheating.
	{
		typedef int Type;

		enum
		{
			HBlank = 0,
			VBlank,
			SpritesLocked,
			VideoRamLocked
		};
	};

	void Begin_HBlank();
	void Begin_VBlank();
	void Begin_SpritesLocked();
	void Begin_VideoRamLocked();

	void Run_VBlank(int ticks);

	DisplayState::Type m_currentState;
	int m_stateTicksRemaining;
	int m_vblankScanlineTicksRemaining;

	void RenderBackgroundPixel(int screenX, int screenY);
	void RenderSpritePixel(int screenX, int screenY);
	void RenderWindowPixel(int screenX, int screenY);

	void RenderSprites(int screenY);

	void RenderPixel(int screenX, int screenY);
	void RenderScanline();
	void Render(int ticks);

	void CheckCoincidence();


	Gameboy* m_machine;
	EmulatedMachine::Type m_machineType;

	Memory* m_memory;

	GameboyScreenBuffer m_screenBuffer;
	GameboyScreenBuffer m_screenBuffer2;
	GameboyScreenBuffer* m_activeScreenBuffer;	///<The screen buffer currently being rendered to by the gameboy
	GameboyScreenBuffer* m_stableScreenBuffer;	///<The screen buffer ready to be displayed on the pc
	int m_screenBufferCount;

	GameboyScreenBuffer m_screenBufferCopy;
	int m_screenBufferCopyId;
	std::mutex m_screenBufferLock;

	DisplayPixel m_displayPalette[4];	///<Maps 2-bit pixel values to DisplayPixel values

	int m_nextPixelToRenderX;	///<X-position of the last pixel rendered during this scanline
	int m_ticksSpentThisScanline;	///<How many ticks have passed during this scanline.  So we know how many pixels to render.
	bool m_spriteHasPriority[160];	///<Sprites are rendered before the background and window, so we need to keep track of whether they should be on top.


	// Caches

	u16 m_vramOffset;
	u16 m_oamOffset;


	// Background data

	GameboyScreenBuffer m_frameBackgroundData;


	// Window data

	GameboyScreenBuffer m_frameWindowData;


	// Sprite data

	GameboyScreenBuffer m_frameSpriteData;


	// Tile data

	void UpdateTileData(int bank, u16 address, u8 value);

	u8 m_tileData[2][(8*8) * (0x1800/16)];	///<0x8000 - 0x97ff


	// Registers

	u8 m_lcdControl;
	u8 m_lcdStatus;

	u8 m_scrollY;
	u8 m_scrollX;

	u8 m_currentScanline;
	u8 m_scanlineCompare;

	u8 m_backgroundPalette;
	u8 m_spritePalette0;
	u8 m_spritePalette1;

	u8 m_windowX;
	u8 m_windowY;

	u16 m_cgbBackgroundPaletteColor[8 * 4];	///<8 palettes, 4 colors per palette
	u8 m_cgbBackgroundPaletteIndex;
	u8 m_cgbBackgroundPaletteData;
	DisplayPixel m_cgbBackgroundDisplayColor[8 * 4];	///<cgb->rgb converted version of backgroundPaletteColor

	u16 m_cgbSpritePaletteColor[8 * 4];	///<8 palettes, 4 colors per palette
	u8 m_cgbSpritePaletteIndex;
	u8 m_cgbSpritePaletteData;
	DisplayPixel m_cgbSpriteDisplayColor[8 * 4];	///<cgb->rgb converted version of spritePaletteColor


	// Properties from registers

	bool m_lcdEnabled;
};

}	//namespace Emunisce

#endif
