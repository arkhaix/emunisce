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

	static u16 pc;
	static u16 sp;

	static u16 af;
	static u16 bc;
	static u16 de;
	static u16 hl;

	static u8& a;
	static u8& f;
	static u8& b;
	static u8& c;
	static u8& d;
	static u8& e;
	static u8& h;
	static u8& l;

	static bool iff1;
	static bool iff2;
	static bool delayInterrupts;	//??? unneeded?  thinking it should be used after/during DI, EI?

	static Memory* memory;

	static void Initialize();
	static void Reset();

	static int Execute();


private:

	static int	ExecuteCB(u8 opcode);

	/*
	static void AddrImmediate(u8* op1);

	static void AddrImmediateExtended(u16* op1);
	//static void AddrImmediateExtended(u8* op1, u8* op2);
	*/


	//Can't these be reduced?  Should never need to use address?  Done by addressing part of opcode?

	static void ExecADC(u8* target, u8 value);
	static void ExecADC(u16* target, u16 value);

	static void ExecADD(u8* target, u8 value);
	static void ExecADD(u16* target, u16 value);

	static void ExecAND(u8 value);

	static void ExecBIT(u8 value, int n);

	static void ExecCALL(bool test, u16 address);	//TODO

	static void ExecCCF();

	static void ExecCP(u8 value);

	static void ExecCPL();

	static void ExecDAA();

	static void ExecDEC(u8* target);
	static void ExecDEC(u16* target);
	//static void ExecDEC(u16 address);

	static void ExecDI();

	static void ExecDJNZ(u8 e);	//TODO

	static void ExecEI();

	static void ExecEX(u16* target1, u16* target2);
	//static void ExecEX(u16 address, u16* target2);

	static void ExecHALT();	//TODO

	static void ExecINC(u8* target);
	static void ExecINC(u16* target);
	//static void ExecINC(u16 address);

	static void ExecJP(bool test, u16 address);		//TODO

	static void ExecJR(bool test, u8 value);		//TODO

	static void ExecLD(u8* target, u8 value);
	static void ExecLD(u16* target, u16 value);
	//static void ExecLD(u16 address, u8 value);

	static void ExecNOP();

	static void ExecOR(u8* target);
	static void ExecOR(u8 value);

	static void ExecPOP(u16* target);	//TODO

	static void ExecPUSH(u16* target);	//TODO

	static void ExecRES(u8* target, int n);
	//static void ExecRES(u16 address, int n);

	static void ExecRET(bool test);		//TODO

	static void ExecRL(u8* target);
	//static void ExecRL(u16 address);

	static void ExecRLA();

	static void ExecRLC(u8* target);
	//static void ExecRLC(u16 address);

	static void ExecRLCA();

	static void ExecRR(u8* target);
	//static void ExecRR(u16 address);

	static void ExecRRA();

	static void ExecRRC(u8* target);
	//static void ExecRRC(u16 address);

	static void ExecRRCA();

	static void ExecRST(u16 address);	//TODO

	static void ExecSBC(u8* target, u8 value);
	static void ExecSBC(u16* target, u16 value);

	static void ExecSCF();

	static void ExecSET(u8* target, int n);
	//static void ExecSET(u16 address, int n);

	static void ExecSLA(u8* target);
	//static void ExecSLA(u16 address);

	//SLL?
	//static void ExecSLL(u8* target);	//value?	//??? this doesn't exist?

	static void ExecSRA(u8* target);
	//static void ExecSRA(u16 address);

	//SRL?
	static void ExecSRL(u8* target);	//value?

	static void ExecSUB(u8* target);
	//static void ExecSUB(u16 address);

	static void ExecXOR(u8* target);
	//static void ExecXOR(u16 address);
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
