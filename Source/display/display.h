#ifndef DISPLAY_H
#define DISPLAY_H

#include "../common/types.h"

struct DisplayPixel
{
	u8 Value;
};

struct ScreenBuffer
{
	DisplayPixel Pixels[160*144];

	DisplayPixel& operator()(int x, int y)
	{
		return Pixels[y*160+x];
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

	//0xff40 - LCDC
	u8 GetLcdControl();
	void SetLcdControl(u8 value);

	//0xff41 - STAT
	u8 GetLcdStatus();
	void SetLcdStatus(u8 value);

	//0xff42 - SCY
	u8 GetScrollY();
	void SetScrollY(u8 value);

	//0xff43 - SCX
	u8 GetScrollX();
	void SetScrollX(u8 value);

	//0xff44 - LY
	u8 GetCurrentScanline();
	void SetCurrentScanline(u8 value);

	//0xff45 - LYC
	u8 GetScanlineCompare();
	void SetScanlineCompare(u8 value);

	//0xff47 - BGP
	u8 GetBackgroundPalette();
	void SetBackgroundPalette(u8 value);

	//0xff48 - OBP0
	u8 GetSpritePalette0();
	void SetSpritePalette0(u8 value);

	//0xff49 - OBP1
	u8 GetSpritePalette1();
	void SetSpritePalette1(u8 value);

	//0xff4a - WY
	u8 GetWindowY();
	void SetWindowY(u8 value);

	//0xff4b - WX
	u8 GetWindowX();
	void SetWindowX(u8 value);

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


	// Tile data

	void UpdateTileData(u16 address, u8 value);

	u8 m_tileData[8*8 * 0x1800];	///<0x8000 - 0x97ff


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
