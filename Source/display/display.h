#ifndef DISPLAY_H
#define DISPLAY_H

#include "../common/types.h"

#define PIXEL_NOT_CACHED ((u8)-1)

struct ScreenBuffer
{
	u8 Pixels[160*144];

	u8 GetPixel(int x, int y)
	{
		return Pixels[y*160+x];
	}

	void SetPixel(int x, int y, u8 value)
	{
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
	void Reset();

	void Run(int ticks);


	//Notifications
	void WriteVram(u16 address, u8 value);
	void WriteOam(u16 address, u8 value);


	//External
	ScreenBuffer GetStableScreenBuffer();


	//Gameboy registers

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

	void RenderPixel(int screenX, int screenY);
	void RenderScanline();

	void CheckCoincidence();


	Memory* m_memory;

	ScreenBuffer m_screenBuffer;
	ScreenBuffer m_screenBuffer2;
	ScreenBuffer* m_activeScreenBuffer;	///<The screen buffer currently being rendered to by the gameboy
	ScreenBuffer* m_stableScreenBuffer;	///<The screen buffer ready to be displayed on the pc
	void* m_screenBufferLock;


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
};

#endif
