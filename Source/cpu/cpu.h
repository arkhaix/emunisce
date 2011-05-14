#ifndef CPU_H
#define CPU_H

#include "../common/types.h"

class Memory;

//Flag positions
#define BIT_Z (7)
#define BIT_N (6)
#define BIT_H (5)
#define BIT_C (4)

//Set flag
#define SET_Z (f |= 0x80)
#define SET_N (f |= 0x40)
#define SET_H (f |= 0x20)
#define SET_C (f |= 0x10)

//Reset flag
#define RES_Z (f &= ~0x80)
#define RES_N (f &= ~0x40)
#define RES_H (f &= ~0x20)
#define RES_C (f &= ~0x10)

//Return flag value if set, 0 otherwise
#define VAL_Z ((u8)(f & 0x80))
#define VAL_N ((u8)(f & 0x40))
#define VAL_H ((u8)(f & 0x20))
#define VAL_C ((u8)(f & 0x10))

//Return 1 if flag is set, 0 otherwise
#define TST_Z (u8)(VAL_Z >> BIT_Z)
#define TST_N (u8)(VAL_N >> BIT_N)
#define TST_H (u8)(VAL_H >> BIT_H)
#define TST_C (u8)(VAL_C >> BIT_C)

//Invert flag
#define INV_Z (f ^= (1<<BIT_Z))
#define INV_N (f ^= (1<<BIT_N))
#define INV_H (f ^= (1<<BIT_H))
#define INV_C (f ^= (1<<BIT_C))

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


class CPU
{
public:

	u16 pc;
	u16 sp;

	u16 af;
	u16 bc;
	u16 de;
	u16 hl;

	u8& a;
	u8& f;
	u8& b;
	u8& c;
	u8& d;
	u8& e;
	u8& h;
	u8& l;

	bool ime;	//Interrupt master enable flag
	bool delayInterrupts;	//Interrupts are not enabled until one instruction after EI completes.

	Memory* memory;

	CPU();

	void Initialize();
	void Reset();

	int Execute();


private:

	int optime;
	bool halted;

	u8 ReadNext8();
	u16 ReadNext16();

	int	ExecuteCB();

	void ExecADC(u8* target, u8 value);
	void ExecADC(u16* target, u16 value);

	void ExecADD(u8* target, u8 value);
	void ExecADD(u16* target, u16 value);

	void ExecAND(u8 value);

	void ExecBIT(u8 value, int n);

	void ExecCALL(u16 address);

	void ExecCCF();

	void ExecCP(u8 value);

	void ExecCPL();

	void ExecDAA();

	void ExecDEC(u8* target);
	void ExecDEC(u16* target);

	void ExecDI();

	void ExecEI();

	void ExecEX(u16* target1, u16* target2);

	void ExecHALT();

	void ExecINC(u8* target);
	void ExecINC(u16* target);

	void ExecJP(u16 address);

	void ExecJR(s8 value);

	void ExecLD(u8* target, u8 value);
	void ExecLD(u16* target, u16 value);

	void ExecNOP();

	void ExecOR(u8* target);
	void ExecOR(u8 value);

	void ExecPOP(u16* target);

	void ExecPUSH(u16* target);

	void ExecRES(u8* target, int n);

	void ExecRET();

	void ExecRL(u8* target);

	void ExecRLA();

	void ExecRLC(u8* target);

	void ExecRLCA();

	void ExecRR(u8* target);

	void ExecRRA();

	void ExecRRC(u8* target);

	void ExecRRCA();

	void ExecRST(u16 address);

	void ExecSBC(u8* target, u8 value);
	void ExecSBC(u16* target, u16 value);

	void ExecSCF();

	void ExecSET(u8* target, int n);

	void ExecSLA(u8* target);

	void ExecSRA(u8* target);

	void ExecSRL(u8* target);

	void ExecSUB(u8 value);

	void ExecSWAP(u8* target);

	void ExecXOR(u8 value);
};


/*

 Opcode List
-===========-
		
08		LD   (nn),SP

10		STOP

22		LDI  (HL),A
2A		LDI  A,(HL)

32		LDD  (HL),A
3A		LDD  A,(HL)

D3		<unused>
D9		RETI
DB		<unused>
DD		<unused>

E0		LD   (FF00+n),A
E2		LD   (FF00+C),A
E3		<unused>
E4		<unused>
E8		ADD  SP,dd
EA		LD   (nn),A
EB		<unused>
EC		<unused>
ED		<unused>

F0		LD   A,(FF00+n)
F2		LD   A,(FF00+C)
F4		<unused>
F8		LD   HL,SP+dd
FA		LD   A,(nn)
FC		<unused>
FD		<unused>

CB3X	SWAP r/(HL)


*/

#endif
