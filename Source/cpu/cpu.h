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
#define TST_Z ((u8)(VAL_Z >> BIT_Z))
#define TST_N ((u8)(VAL_N >> BIT_N))
#define TST_H ((u8)(VAL_H >> BIT_H))
#define TST_C ((u8)(VAL_C >> BIT_C))

//Invert flag
#define INV_Z (f ^= (1<<BIT_Z))
#define INV_N (f ^= (1<<BIT_N))
#define INV_H (f ^= (1<<BIT_H))
#define INV_C (f ^= (1<<BIT_C))


class Cpu
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

	
	Cpu();

	//Component
	void SetMachine(Machine* machine);
	void Initialize();

	//External
	int Step();

	bool IsStopped();

	//Registers
	void SetTimerDivider(u8 value);
	void SetTimerControl(u8 value);

private:

	Machine* m_machine;
	Memory* m_memory;

	bool m_masterInterruptsEnabled;	///<Interrupt master enable flag (IME)
	bool m_delayNextInterrupt;	///<Interrupts are not enabled until one instruction after EI completes.

	int m_instructionTime;
	bool m_halted;
	bool m_stopped;

	//Registers
	u8 m_interruptsEnabled;		///<0xffff - Interrupt Enable.  Which interrupts are currently enabled.  Slaves to the IME flag.
	u8 m_interruptFlags;		///<0xff0f - Interrupt Flag.  Which interrupts are currently set.

	u8 m_timerDivider;	///<0xff04 - Timer Divider.
	int m_ticksUntilDividerIncrement;	///<The timer divider increments once every 256 ticks.

	u8 m_timerModulo;	///<0xff06 - Timer Modulo.  This value gets loaded into the timer counter when it overflows.

	u8 m_timerCounter;	///<0xff05 - Timer Counter.
	int m_ticksPerCounterIncrement;
	int m_ticksUntilCounterIncrement;

	u8 m_timerControl;	///<0xff07 - Timer Control.
	bool m_timerEnabled;

	void UpdateTimer(int ticks);

	u8 ReadNext8();
	u16 ReadNext16();

	int	ExecuteCB();

	void ExecADC(u8* target, u8 value);
	void ExecADC(u16* target, u16 value);

	void ExecADD(u8* target, u8 value);
	void ExecADD(u16* target, u16 value);
	void ExecADD(u16* target, s8 value);

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

	void ExecSBC(u8 value);

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
