#ifndef CPU_H
#define CPU_H

#include "../common/types.h"


#define SET_Z (*f |= 0x80)
#define SET_N (*f |= 0x40)
#define SET_H (*f |= 0x20)
#define SET_C (*f |= 0x10)

#define RES_Z (*f &= ~0x80)
#define RES_N (*f &= ~0x40)
#define RES_H (*f &= ~0x20)
#define RES_C (*f &= ~0x10)

#define TST_Z (*f & 0x80)
#define TST_N (*f & 0x40)
#define TST_H (*f & 0x20)
#define TST_C (*f & 0x10)


class CPU
{
public:

	static u16 af;
	static u16 bc;
	static u16 de;
	static u16 hl;
	static u16 sp;
	static u16 pc;

	static u8* a;
	static u8* f;
	static u8* b;
	static u8* c;
	static u8* d;
	static u8* e;
	static u8* h;
	static u8* l;

	static void Initialize();
	static void Reset();


private:

	//Can't these be reduced?  Should never need to use address?  Done by addressing part of opcode?

	static void ExecADC(u8* target, u8 value);
	static void ExecADC(u16* target, u16 value);

	static void ExecADD(u8* target, u8 value);
	static void ExecADD(u16* target, u16 value);

	static void ExecAND(u8 value);

	static void ExecBIT(u8 value, int n);

	static void ExecCALL(bool test, u16 address);

	static void ExecCCF();

	static void ExecCP(u8 value);

	static void ExecCPL();

	static void ExecDAA();

	static void ExecDEC(u8* target);
	static void ExecDEC(u16* target);
	//static void ExecDEC(u16 address);

	static void ExecDI();

	static void ExecDJNZ(u8 e);

	static void ExecEI();

	static void ExecEX(u16* target1, u16* target2);
	//static void ExecEX(u16 address, u16* target2);

	static void ExecEXX();

	static void ExecHALT();

	static void ExecINC(u8* target);
	static void ExecINC(u16* target);
	//static void ExecINC(u16 address);

	static void ExecJP(bool test, u16 address);

	static void ExecJR(bool test, u8 value);

	static void ExecLD(u8* target, u8 value);
	static void ExecLD(u16* target, u16 value);
	//static void ExecLD(u16 address, u8 value);

	static void ExecNOP();

	static void ExecOR(u8* target);
	static void ExecOR(u8 value);

	static void ExecPOP(u16* target);

	static void ExecPUSH(u16* target);

	static void ExecRES(u8* target, int n);
	//static void ExecRES(u16 address, int n);

	static void ExecRET(bool test);

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

	static void ExecRST(u16 address);

	static void ExecSBC(u8* target, u8 value);
	static void ExecSBC(u16* target, u16 value);

	static void ExecSCF();

	static void ExecSET(u8* target, int n);
	//static void ExecSET(u16 address, int n);

	static void ExecSLA(u8* target);
	//static void ExecSLA(u16 address);

	//SLL?
	static void ExecSLL(u8* target);	//value?

	static void ExecSRA(u8* target);
	//static void ExecSRA(u16 address);

	//SRL?
	static void ExecSRL(u8* target);	//value?

	static void ExecSUB(u8* target);
	//static void ExecSUB(u16 address);

	static void ExecXOR(u8* target);
	//static void ExecXOR(u16 address);
};

#endif
