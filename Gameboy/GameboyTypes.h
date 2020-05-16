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
#ifndef GAMEBOYTYPES_H
#define GAMEBOYTYPES_H


namespace Emunisce
{

	//Component forward-declarations
	class Gameboy;
	class Cpu;
	class Memory;
	class Display;
	class Input;
	class Sound;


	//Registers
#define REG_P1 (0xff00) //Joypad info and system type
#define REG_SB (0xff01) //Serial transfer data
#define REG_SC (0xff02) //Serial I/O control
#define REG_DIV (0xff04) //Divider
#define REG_TIMA (0xff05) //Timer counter
#define REG_TMA (0xff06) //Timer modulo
#define REG_TAC (0xff07) //Timer control
#define REG_IF (0xff0f) //Interrupt flags
#define REG_LCDC (0xff40) //LCD control
#define REG_STAT (0xff41) //LCD status
#define REG_SCY (0xff42) //Scroll Y
#define REG_SCX (0xff43) //Scroll X
#define REG_LY (0xff44) //LCD Y coordinate
#define REG_LYC (0xff45) //LY compare
#define REG_DMA (0xff46) //DMA transfer and start address
#define REG_BGP (0xff47) //Background palette data
#define REG_OBP0 (0xff48) //Object palette 0 data
#define REG_OBP1 (0xff49) //Object palette 1 data
#define REG_WY (0xff4a) //Window Y position
#define REG_WX (0xff4b) //Window X position
#define REG_IE (0xffff) //Interrupt enable flags

#define IF_VBLANK (1<<0) //VBlank flag
#define IF_LCDC (1<<1) //LCD flag
#define IF_TIMER (1<<2) //Timer overflow flag
#define IF_SERIAL (1<<3) //Serial I/O transfer complete flag
#define IF_INPUT (1<<4)	//"Transition from high to low of pin P10-P13".  Think this triggers on any input.

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


}	//namespace Emunisce

#endif
