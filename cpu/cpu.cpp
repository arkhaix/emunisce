#include "cpu.h"

#include "../memory/memory.h"


u16 CPU::af;
u16 CPU::bc;
u16 CPU::de;
u16 CPU::hl;
u16 CPU::sp;
u16 CPU::pc;

u8& CPU::a = *(((u8*)&af)+1);
u8& CPU::f = *(((u8*)&af)+0);
u8& CPU::b = *(((u8*)&bc)+1);
u8& CPU::c = *(((u8*)&bc)+0);
u8& CPU::d = *(((u8*)&de)+1);
u8& CPU::e = *(((u8*)&de)+0);
u8& CPU::h = *(((u8*)&hl)+1);
u8& CPU::l = *(((u8*)&hl)+0);

//TODO: initial values for iff1, iff2, canInterrupt
bool CPU::iff1 = false;
bool CPU::iff2 = false;
bool CPU::delayInterrupts = false;

Memory* CPU::memory = 0;


void CPU::Initialize()
{
	
}

void CPU::Reset()
{
	Initialize();
}


//////////////////////////////////////////////////////////////////


int CPU::Execute()
{
	return 0;
}

int ExecuteCB(u8 opcode)
{
	return 0;
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
		SET_Z;
	else
		RES_Z;

	//N
	RES_N;

	//H
	SET_H;

	//C unaffected
}

void CPU::ExecCALL(bool test, u16 address)
{

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
	iff1 = false;
	iff2 = false;

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecDJNZ(u8 e)
{

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecEI()
{
	iff1 = true;
	iff2 = true;
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

void CPU::ExecJP(bool test, u16 address)
{
	//TODO
	
	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecJR(bool test, u8 value)
{
	//TODO

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
	//TODO
}

void CPU::ExecPUSH(u16* target)
{
	//TODO
}

void CPU::ExecRES(u8* target, int n)
{
	*target &= ~(1<<n);

	//Z unaffected

	//N unaffected

	//H unaffected

	//C unaffected
}

void CPU::ExecRET(bool test)
{
	//TODO
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
	//TODO
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

void CPU::ExecSUB(u8* target)
{
	int res = a - *target;

	//Z
	if(res == 0)
		SET_Z;
	else
		RES_Z;

	//N
	SET_N;

	//H
	if( (a ^ *target ^ res) & 0x10 )
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

void CPU::ExecXOR(u8* target)
{
	a ^= *target;

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
