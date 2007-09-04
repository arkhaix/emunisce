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


void CPU::Initialize()
{
	
}

void CPU::Reset()
{
	Initialize();
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
	if( (res & 0x10) && !(*target & 0x10) )
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

	//H unaffected

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
	if( (res & 0x10) && !(*target & 0x10) )
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

	//H unaffected

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

void CPU::ExecCALL(bool test, u16 address)	//TODO
{
	if(test)
		pc = address;
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
	if( !(res & 0x10) && (value & 0x10) )
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
