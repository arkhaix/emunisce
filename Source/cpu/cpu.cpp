#include "cpu.h"

#include "../memory/memory.h"


CPU::CPU()
: a(*(((u8*)&af)+1))
, f(*(((u8*)&af)+0))
, b(*(((u8*)&bc)+1))
, c(*(((u8*)&bc)+0))
, d(*(((u8*)&de)+1))
, e(*(((u8*)&de)+0))
, h(*(((u8*)&hl)+1))
, l(*(((u8*)&hl)+0))
{
	Initialize();
}

void CPU::Initialize()
{
	memory = 0;
	Reset();
}

void CPU::Reset()
{
	ime = false;
	delayInterrupts = false;

	optime = 0;
	halted = false;


	af = 0x01b0;
	bc = 0x0013;
	de = 0x00d8;
	hl = 0x014d;

	sp = 0xfffe;

	pc = 0x0100;
}


u8 CPU::ReadNext8()
{
	u8 result = memory->Read8(pc);
	pc++;
	return result;
}

u16 CPU::ReadNext16()
{
	u16 result = memory->Read16(pc);
	pc += 2;
	return result;
}


//////////////////////////////////////////////////////////////////



void CPU::ExecADC(u8* target, u8 value)
{
	int res = *target + value + TST_C;
	
	//Z
	if((res & 0xff) == 0) 
		SET_Z;
	else 
		RES_Z;

	//N
	RES_N;

	//H
	if( (*target ^ value ^ res) & 0x10 )
		SET_H;
	else
		RES_H;

	//C
	if(res & 0x100) 
		SET_C;
	else 
		RES_C;

	*target = (u8)res;
}

void CPU::ExecADC(u16* target, u16 value)
{
	int res = *target + value + TST_C;

	//Z
	if((res & 0xffff) == 0) 
		SET_Z;
	else 
		RES_Z;

	//N
	RES_N;

	//H
	if( (*target ^ value ^ res) & 0x1000 )
		SET_H;
	else
		RES_H;

	//C
	if(res & 0x10000) 
		SET_C;
	else 
		RES_C;

	*target = (u16)res;
}

void CPU::ExecADD(u8* target, u8 value)
{
	int res = *target + value;
	
	//Z
	if((res & 0xff) == 0) 
		SET_Z;
	else 
		RES_Z;

	//N
	RES_N;

	//H
	if( (*target ^ value ^ res) & 0x10 )
		SET_H;
	else
		RES_H;

	//C
	if(res & 0x100) 
		SET_C;
	else 
		RES_C;

	*target = (u8)res;
}

void CPU::ExecADD(u16* target, u16 value)
{
	int res = *target + value;

	//Z
	if((res & 0xffff) == 0) 
		SET_Z;
	else 
		RES_Z;

	//N
	RES_N;

	//H
	if( (*target ^ value ^ res) & 0x1000 )
		SET_H;
	else
		RES_H;

	//C
	if(res & 0x10000) 
		SET_C;
	else 
		RES_C;

	*target = (u16)res;
}

void CPU::ExecAND(u8 value)
{
	a = a & value;

	//Z
	if(a == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	SET_H;

	//C
	RES_C;
}

void CPU::ExecBIT(u8 value, int n)
{
	//Z
	if(value & (1<<n))
		RES_Z;
	else
		SET_Z;

	//N
	RES_N;

	//H
	SET_H;

	//C unaffected
}

void CPU::ExecCALL(u16 address)
{
	//(SP-1)<-PCH
	sp--;
	memory->Write8(sp, (u8)((pc & 0xff00) >> 8));

	//(SP-2)<-PCL
	sp--;
	memory->Write8(sp, (u8)(pc & 0x00ff));

	//PC<-nn
	pc = address;

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecCCF()
{
	//Z unaffected

	//N
	RES_N;

	//H
	if(TST_C)
		SET_H;
	else
		RES_H;
	
	//C
	INV_C;	
}

void CPU::ExecCP(u8 value)
{
	int res = a - value;

	//Z
	if(res == 0)
		SET_Z;
	else
		RES_Z;

	//N
	SET_N;

	//H
	//???
	if( (a ^ value ^ res) & 0x10 )
		SET_H;
	else
		RES_H;

	//C
	//???
	if(res < 0)
		SET_C;
	else
		RES_C;
}

void CPU::ExecCPL()
{
	a ^= 0xff;

	//Z unaffected

	//N
	SET_N;

	//H
	SET_H;

	//C unaffected
}

void CPU::ExecDAA()
{
	if(TST_N)
	{
		if(TST_C)
		{
			if(TST_H)
			{
				a += 0x9a;
				SET_C;
			}
			else
			{
				a += 0xa0;
				SET_C;
			}
		}
		else
		{
			if(TST_H)
			{
				a += 0xfa;
				RES_C;
			}
			else
			{
				RES_C;
			}
		}
	}
	else
	{
		if(TST_C)
		{
			if(TST_H)
			{
				a += 0x66;
				SET_C;
			}
			else
			{
				if((a & 0x0f) >= 0x0a)
				{
					a += 0x66;
					SET_C;
				}
				else
				{
					a += 0x60;
					SET_C;
				}
			}
		}
		else
		{
			if(TST_H)
			{
				if((a & 0xf0) >= 0xa0)
				{
					a += 0x66;
					SET_C;
				}
				else
				{
					a += 0x06;
					RES_C;
				}
			}
			else
			{
				if((a & 0x0f) >= 0x0a)
				{
					if((a & 0xf0) >= 0x90)
					{
						a += 0x66;
						SET_C;
					}
					else
					{
						a += 0x06;
						RES_C;
					}
				}
				else
				{
					if((a & 0xf0) >= 0x0a)
					{
						a += 0x60;
						SET_C;
					}
					else
					{
						RES_C;
					}
				}
			}
		}
	}


	//Z
	if(a == 0)
		SET_Z;
	else
		RES_Z;

	//N unaffected

	//H
	//???
	//unaffected?

	//C set above
}

void CPU::ExecDEC(u8* target)
{
	u8 res = (*target) - 1;

	//Z
	if(res == 0)
		SET_Z;
	else
		RES_Z;

	//N
	SET_N;

	//H
	//??? VBA disagrees
	if( (*target & 0x10) && !(res & 0x10) )
		SET_H;
	else
		RES_H;

	//C unaffected

	*target = res;
}

void CPU::ExecDEC(u16* target)
{
	*target = (*target) - 1;

	//???
	// Are these really unaffected?  Doesn't make sense.

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecDI()
{
	ime = false;

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecEI()
{
	ime = true;
	delayInterrupts = true;

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecEX(u16* target1, u16* target2)
{
	u16 tmp = *target1;
	*target1 = *target2;
	*target2 = tmp;

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecHALT()
{
	halted = true;

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecINC(u8* target)
{
	u8 res = *target + 1;

	//Z
	if(*target == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	if( !(*target & 0x10) && (res & 0x10) )
		SET_H;
	else
		RES_H;

	//C unaffected

	*target = res;
}

void CPU::ExecINC(u16* target)
{
	*target = (*target) + 1;

	//???
	// Are these really unaffected?  Doesn't make sense.

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecJP(u16 address)
{
	pc = address;
	
	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecJR(s8 value)
{
	pc += value;

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecLD(u8* target, u8 value)
{
	*target = value;

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecLD(u16* target, u16 value)
{
	*target = value;

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecNOP()
{
	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecOR(u8* target)
{
	a |= *target;

	//Z
	if(a == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C
	RES_C;
}

void CPU::ExecOR(u8 value)
{
	a |= value;

	//Z
	if(a == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C
	RES_C;
}

void CPU::ExecPOP(u16* target)
{
	//L<-(SP)
	*target = memory->Read8(sp);

	//H<-(SP+1)
	sp++;
	*target |= (memory->Read8(sp) << 8);

	sp++;


	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecPUSH(u16* target)
{
	//(SP-1)<-H
	sp--;
	memory->Write8(sp, (u8)(*target >> 8));

	//(SP-2)<-L
	sp--;
	memory->Write8(sp, (u8)(*target & 0x00ff));


	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecRES(u8* target, int n)
{
	*target &= ~(1<<n);

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecRET()
{
	//PCL<-(SP)
	pc = memory->Read8(sp);

	//PCH<-(SP+1)
	sp++;
	pc |= (memory->Read8(sp) << 8);

	sp++;


	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecRL(u8* target)
{
	int oldC = TST_C;

	if(*target & 0x80)
		SET_C;
	else
		RES_C;

	*target <<= 1;
	*target |= oldC;

	//Z
	if(*target == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;
	
	//C handled above
}

void CPU::ExecRLA()
{
	int oldC = TST_C;

	if(a & 0x80)
		SET_C;
	else
		RES_C;

	a <<= 1;
	a |= oldC;

	//Z unaffected

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecRLC(u8* target)
{
	if(*target & 0x80)
		SET_C;
	else
		RES_C;

	*target <<= 1;
	*target |= TST_C;

	//Z
	if(*target == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecRLCA()
{
	if(a & 0x80)
		SET_C;
	else
		RES_C;

	a <<= 1;
	a |= TST_C;

	//Z
	if(a == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecRR(u8* target)
{
	int oldC = TST_C;

	if(*target & 0x01)
		SET_C;
	else
		RES_C;

	*target >>= 1;
	*target |= (oldC << 7);

	//Z
	if(*target == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecRRA()
{
	int oldC = TST_C;

	if(a & 0x01)
		SET_C;
	else
		RES_C;

	a >>= 1;
	a |= (oldC << 7);

	//Z unaffected

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecRRC(u8* target)
{
	if(*target & 0x01)
		SET_C;
	else
		RES_C;

	*target >>= 1;
	*target |= (TST_C << 7);

	//Z
	if(*target == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecRRCA()
{
	if(a & 0x01)
		SET_C;
	else
		RES_C;

	a >>= 1;
	a |= (TST_C << 7);

	//Z unaffected

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecRST(u16 address)
{
	ExecCALL(address);
}

void CPU::ExecSBC(u8* target, u8 value)
{
	int res = *target - value - TST_C;

	//Z
	if(res == 0)
		SET_Z;
	else
		RES_Z;

	//N
	SET_N;

	//H
	if( (*target ^ value ^ res) & 0x10 )
		SET_H;
	else
		RES_H;

	//C
	if(res < 0)
		SET_C;
	else
		RES_C;

	*target = (u8)res;
}

void CPU::ExecSBC(u16* target, u16 value)
{
	int res = *target - value - TST_C;

	//Z
	if(res == 0)
		SET_Z;
	else
		RES_Z;

	//N
	SET_N;

	//H
	if( (*target ^ value ^ res) & 0x1000 )
		SET_H;
	else
		RES_H;

	//C
	if(res < 0)
		SET_C;
	else
		RES_C;

	*target = (u8)res;
}

void CPU::ExecSCF()
{
	//Z unaffected

	//N
	RES_N;

	//H
	RES_H;

	//C
	SET_C;
}

void CPU::ExecSET(u8* target, int n)
{
	*target |= (1<<n);

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecSLA(u8* target)
{
	if(*target & 0x80)
		SET_C;
	else
		RES_C;

	*target <<= 1;

	//Z
	if(*target == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecSRA(u8* target)
{
	int bit7 = (*target & 0x80);

	if(*target & 0x01)
		SET_C;
	else
		RES_C;

	*target >>= 1;
	*target |= bit7;

	//Z
	if(*target == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecSRL(u8* target)
{
	if(*target & 0x01)
		SET_C;
	else
		RES_C;

	*target >>= 1;

	//Z
	if(*target == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C handled above
}

void CPU::ExecSUB(u8 value)
{
	int res = a - value;

	//Z
	if(res == 0)
		SET_Z;
	else
		RES_Z;

	//N
	SET_N;

	//H
	if( (a ^ value ^ res) & 0x10 )
		SET_H;
	else
		RES_H;

	//C
	if(res < 0)
		SET_C;
	else
		RES_C;

	a = (u8)res;
}

void CPU::ExecSWAP(u8* target)
{
	u8 low = (*target) & 0x0f;
	*target >>= 4;
	*target |= (low<<4);

	//Z
	//??

	//N
	//??

	//H
	//??

	//C
	//??
}

void CPU::ExecXOR(u8 value)
{
	a ^= value;

	//Z
	if(a == 0)
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	RES_H;

	//C
	RES_C;
}
