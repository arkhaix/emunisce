#ifndef TYPES_H
#define TYPES_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#ifndef NULL
#define NULL 0
#endif

//Component forward-declarations
class Machine;
class Cpu;
class Display;
class Input;
class Memory;

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

#endif
