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

	bool iff1;
	bool iff2;
	bool delayInterrupts;	//??? unneeded?  thinking it should be used after/during DI, EI?

	Memory* memory;

	CPU();

	void Initialize();
	void Reset();

	int Execute();


private:

	int optime;
	bool halted;

	int	ExecuteCB();

	/*
	void AddrImmediate(u8* op1);

	void AddrImmediateExtended(u16* op1);
	//void AddrImmediateExtended(u8* op1, u8* op2);
	*/


	//Can't these be reduced?  Should never need to use address?  Done by addressing part of opcode?

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
	//void ExecDEC(u16 address);

	void ExecDI();

	void ExecDJNZ(s8 e);

	void ExecEI();

	void ExecEX(u16* target1, u16* target2);
	//void ExecEX(u16 address, u16* target2);

	void ExecHALT();

	void ExecINC(u8* target);
	void ExecINC(u16* target);
	//void ExecINC(u16 address);

	void ExecJP(u16 address);

	void ExecJR(s8 value);

	void ExecLD(u8* target, u8 value);
	void ExecLD(u16* target, u16 value);
	//void ExecLD(u16 address, u8 value);

	void ExecNOP();

	void ExecOR(u8* target);
	void ExecOR(u8 value);

	void ExecPOP(u16* target);

	void ExecPUSH(u16* target);

	void ExecRES(u8* target, int n);
	//void ExecRES(u16 address, int n);

	void ExecRET();

	void ExecRL(u8* target);
	//void ExecRL(u16 address);

	void ExecRLA();

	void ExecRLC(u8* target);
	//void ExecRLC(u16 address);

	void ExecRLCA();

	void ExecRR(u8* target);
	//void ExecRR(u16 address);

	void ExecRRA();

	void ExecRRC(u8* target);
	//void ExecRRC(u16 address);

	void ExecRRCA();

	void ExecRST(u16 address);

	void ExecSBC(u8* target, u8 value);
	void ExecSBC(u16* target, u16 value);

	void ExecSCF();

	void ExecSET(u8* target, int n);
	//void ExecSET(u16 address, int n);

	void ExecSLA(u8* target);
	//void ExecSLA(u16 address);

	//SLL?
	//void ExecSLL(u8* target);	//value?	//??? this doesn't exist?

	void ExecSRA(u8* target);
	//void ExecSRA(u16 address);

	//SRL?
	void ExecSRL(u8* target);	//value?

	void ExecSUB(u8* target);
	//void ExecSUB(u16 address);

	void ExecXOR(u8* target);
	//void ExecXOR(u16 address);
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
