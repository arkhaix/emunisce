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
#ifndef DISPLAY_H
#define DISPLAY_H

#include "../Machine/Types.h"

#define PIXEL_NOT_CACHED (DisplayPixelFromRGBA((u8)0, (u8)0, (u8)0, (u8)1))
#define PIXEL_TRANSPARENT (DisplayPixelFromRGBA((u8)0, (u8)0, (u8)0, (u8)0))

struct ScreenResolution
{
	int width;
	int height;
};

struct ScreenBuffer
{
	DisplayPixel Pixels[160*144];

	inline DisplayPixel GetPixel(int x, int y)
	{
		if(x<0 || x>=160 || y<0 || y>=144)
			return (DisplayPixel)0;

		return Pixels[y*160+x];
	}

	inline void SetPixel(int x, int y, DisplayPixel value)
	{
		if(x<0 || x>=160 || y<0 || y>=144)
			return;

		Pixels[y*160+x] = value;
	}
};

class Display
{
public:

	Display();
	~Display();


	//Component
	void SetMachine(Machine* machine);
	void Initialize();

	void Run(int ticks);


	//Notifications
	void WriteVram(u16 address, u8 value);
	void WriteOam(u16 address, u8 value);


	//External
	ScreenResolution GetScreenResolution();
	ScreenBuffer GetStableScreenBuffer();
	int GetScreenBufferCount();	///<Returns the id of the current screen buffer.  Not guaranteed to be unique or sequential, so use != when polling for changes.


	//Gameboy registers

	void SetLcdControl(u8 value);
	void SetLcdStatus(u8 value);
	void SetCurrentScanline(u8 value);
	void SetScanlineCompare(u8 value);


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


	Machine* m_machine;
	Memory* m_memory;

	ScreenBuffer m_screenBuffer;
	ScreenBuffer m_screenBuffer2;
	ScreenBuffer* m_activeScreenBuffer;	///<The screen buffer currently being rendered to by the gameboy
	ScreenBuffer* m_stableScreenBuffer;	///<The screen buffer ready to be displayed on the pc
	int m_screenBufferCount;

	DisplayPixel m_displayPalette[4];	///<Maps 2-bit pixel values to DisplayPixel values

	int m_nextPixelToRenderX;	///<X-position of the last pixel rendered during this scanline
	int m_ticksSpentThisScanline;	///<How many ticks have passed during this scanline.  So we know how many pixels to render.
	bool m_spriteHasPriority[160];	///<Sprites are rendered before the background and window, so we need to keep track of whether they should be on top.


	// Caches

	u8 m_vramCache[0x2000];
	u16 m_vramOffset;

	u8 m_oamCache[0xa0];
	u16 m_oamOffset;


	// Background data

	ScreenBuffer m_frameBackgroundData;


	// Window data

	ScreenBuffer m_frameWindowData;


	// Sprite data

	ScreenBuffer m_frameSpriteData;


	// Tile data

	void UpdateTileData(u16 address, u8 value);

	u8 m_tileData[(8*8) * (0x1800/16)];	///<0x8000 - 0x97ff


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


	// Properties from registers

	bool m_lcdEnabled;
};

#endif
