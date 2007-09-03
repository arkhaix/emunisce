#include "cpu.h"

#include "../memory/memory.h"


u16 CPU::af;
u16 CPU::bc;
u16 CPU::de;
u16 CPU::hl;
u16 CPU::sp;
u16 CPU::pc;

u8* CPU::a;
u8* CPU::f;
u8* CPU::b;
u8* CPU::c;
u8* CPU::d;
u8* CPU::e;
u8* CPU::h;
u8* CPU::l;


void CPU::Initialize()
{
	a=((u8*)(&af))+1;
	f=((u8*)(&af))+0;
	b=((u8*)(&bc))+1;
	c=((u8*)(&bc))+0;
	d=((u8*)(&de))+1;
	e=((u8*)(&de))+0;
	h=((u8*)(&hl))+1;
	l=((u8*)(&hl))+0;
}

void CPU::Reset()
{
	Initialize();
}
