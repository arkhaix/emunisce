#include "cpu.h"

#include "../memory/memory.h"


int CPU::Execute()
{
	optime = 0;

	u8 n,t;
	u16 nn,tt;
	u16 address;

	u8 opcode;
	
	if(halted)
	{
		opcode = 0x00;	//When halted, we just execute NOPs until an interrupt
	}
	else
	{
		opcode = memory->Read8(pc);
		pc++;
	}

	//CB xx
	if(opcode == 0xcb)
	{
		return ExecuteCB();
	}

	switch(opcode)
	{


	// 0x


	case 0x00:
		//NOP
		ExecNOP();
		optime += 4;
	break;
	
	case 0x01:
		//01 n n		LD BC,nn		10	3	1
		nn = ReadNext16();
		ExecLD(&bc, nn);
		optime += 10;
	break;

	case 0x02:
		//02		LD (BC),A		7	2	1
		ExecLD(&t, a);
		memory->Write8(bc, t);
		optime += 7;
	break;

	case 0x03:
		//03		INC BC			6	1	1
		ExecINC(&bc);
		optime += 6;
	break;

	case 0x04:
		//04		INC B			4	1	1
		ExecINC(&b);
		optime += 4;
	break;

	case 0x05:
		//05		DEC B			4	1	1
		ExecDEC(&b);
		optime += 4;
	break;

	case 0x06:
		//06 n		LD B,n			7	2	1
		n = ReadNext8();
		ExecLD(&b, n);
		optime += 7;
	break;

	case 0x07:
		//07		RLCA			4	1	1
		ExecRLCA();
		optime += 4;
	break;

	case 0x08:
		//08      EX   AF,AF      LD   (nn),SP
		nn = ReadNext16();
		ExecLD(&tt, sp);
		memory->Write16(nn, tt);
		optime += 16;	//based on LD (nn),HL
	break;

	case 0x09:
		//09		ADD HL,BC		11	3	1
		ExecADD(&hl, bc);
		optime += 11;
	break;

	case 0x0a:
		//0A		LD A,(BC)		7	2	1
		ExecLD(&a, memory->Read8(bc));
		optime += 7;
	break;

	case 0x0b:
		//0B		DEC BC			6	1	1
		ExecDEC(&bc);
		optime += 6;
	break;

	case 0x0c:
		//0C		INC C			4	1	1
		ExecINC(&c);
		optime += 4;
	break;

	case 0x0d:
		//0D		DEC C			4	1	1
		ExecDEC(&c);
		optime += 4;
	break;

	case 0x0e:
		//0E n		LD C,n			7	2	1
		n = ReadNext8();
		ExecLD(&c, n);
		optime += 7;
	break;

	case 0x0f:
		//0F		RRCA			4	1	1
		ExecRRCA();
		optime += 4;
	break;


	// 1x


	case 0x10:
		//10      DJNZ PC+dd      STOP
		//??
		halted = true;
	break;

	case 0x11:
		//11 n n		LD DE,nn		10	3	1
		nn = ReadNext16();
		ExecLD(&de, nn);
		optime += 10;
	break;

	case 0x12:
		//12		LD (DE),A		7	2	1
		ExecLD(&t, a);
		memory->Write8(de, t);
		optime += 7;
	break;

	case 0x13:
		//13		INC DE			6	1	1
		ExecINC(&de);
		optime += 6;
	break;

	case 0x14:
		//14		INC D			4	1	1
		ExecINC(&d);
		optime += 4;
	break;

	case 0x15:
		//15		DEC D			4	1	1
		ExecDEC(&d);
		optime += 4;
	break;

	case 0x16:
		//16 n		LD D,n			7	2	1
		n = ReadNext8();
		ExecLD(&d, n);
		optime += 7;
	break;

	case 0x17:
		//17		RLA			4	1	1
		ExecRLA();
		optime += 4;
	break;

	case 0x18:
		//18 e		JR (PC+e)		12	3	1
		n = ReadNext8();
		ExecJR((s8)n);
		optime += 12;
	break;

	case 0x19:
		//19		ADD HL,DE		11	3	1
		ExecADD(&hl, de);
		optime += 11;
	break;

	case 0x1a:
		//1A		LD A,(DE)		7	2	1
		n = memory->Read8(de);
		ExecLD(&a, n);
		optime += 7;
	break;

	case 0x1b:
		//1B		DEC DE			6	1	1
		ExecDEC(&de);
		optime += 6;
	break;

	case 0x1c:
		//1C		INC E			4	1	1
		ExecINC(&e);
		optime += 4;
	break;

	case 0x1d:
		//1D		DEC E			4	1	1
		ExecDEC(&e);
		optime += 4;
	break;

	case 0x1e:
		//1E n		LD E,n			7	2	1
		n = ReadNext8();
		ExecLD(&e, n);
		optime += 7;
	break;

	case 0x1f:
		//1F		RRA			4	1	1
		ExecRRA();
		optime += 4;
	break;


	// 2x


	case 0x20:
		//20 e		JR NZ,(PC+e)		12/7	3/2	1/1	(met/not met)
		n = ReadNext8();
		if(TST_Z)
		{
			optime += 7;
		}
		else
		{
			ExecJR((s8)n);
			optime += 12;
		}
	break;

	case 0x21:
		//21 n n		LD HL,nn		10	3	1
		nn = ReadNext16();
		ExecLD(&hl, nn);
		optime += 10;
	break;

	case 0x22:
		//22      LD   (nn),HL    LD  (HL+),A
		ExecLD(&n, a);
		memory->Write8(hl, n);
		ExecINC(&hl);
		optime += 9; //?? Based on Fetch/Decode = 3, INC HL = 6 (guessing 3+3), and LD (HL), A = (4+3) (guessing 3+1 +3)
	break;

	case 0x23:
		//23		INC HL			6	1	1
		ExecINC(&hl);
		optime += 6;
	break;

	case 0x24:
		//24		INC H			4	1	1
		ExecINC(&h);
		optime += 4;
	break;

	case 0x25:
		//25		DEC H			4	1	1
		ExecDEC(&h);
		optime += 4;
	break;

	case 0x26:
		//26 n		LD H,n			7	2	1
		n = ReadNext8();
		ExecLD(&h, n);
		optime += 7;
	break;

	case 0x27:
		//27		DAA			4	1	1
		ExecDAA();
		optime += 4;
	break;

	case 0x28:
		//28 e		JR Z,(PC+e)		12/7	3/2	1/1	(met/not met)
		n = ReadNext8();
		if(TST_Z)
		{
			ExecJR((s8)n);
			optime += 12;
		}
		else
		{
			optime += 7;
		}
	break;

	case 0x29:
		//29		ADD HL,HL		11	3	1
		ExecADD(&hl, hl);
		optime += 11;
	break;

	case 0x2a:
		//2A      LD   HL,(nn)    LD  A,(HL+)
		n = memory->Read8(hl);
		ExecLD(&a, n);
		ExecINC(&hl);
		optime += 9;	//?? based on 0x22. check this.
	break;

	case 0x2b:
		//2B		DEC HL			6	1	1
		ExecDEC(&hl);
		optime += 6;
	break;

	case 0x2c:
		//2C		INC L			4	1	1
		ExecINC(&l);
		optime += 4;
	break;

	case 0x2d:
		//2D		DEC L			4	1	1
		ExecDEC(&l);
		optime += 4;
	break;

	case 0x2e:
		//2E n		LD L,n			7	2	1
		n = ReadNext8();
		ExecLD(&l, n);
		optime += 7;
	break;

	case 0x2f:
		//2F		CPL			4	1	1
		ExecCPL();
		optime += 4;
	break;


	// 3x


	case 0x30:
		//30 e		JR NC,(PC+e)		12/7	3/2	1/1	(met/not met)
		n = ReadNext8();
		if(TST_C)
		{
			optime += 7;
		}
		else
		{
			ExecJR((s8)n);
			optime += 12;
		}
	break;

	case 0x31:
		//31 n n		LD SP,nn		10	3	1
		nn = ReadNext16();
		ExecLD(&sp, nn);
		optime += 10;
	break;

	case 0x32:
		//32      LD   (nn),A     LD  (HL-),A
		ExecLD(&n, a);
		memory->Write8(hl, n);
		ExecDEC(&hl);
		optime += 9;	//?? based on 0x22.
	break;

	case 0x33:
		//33		INC SP			6	1	1
		ExecINC(&sp);
		optime += 6;
	break;

	case 0x34:
		//34		INC (HL)		11	3	1
		n = memory->Read8(hl);
		ExecINC(&n);
		memory->Write8(hl, n);
		optime += 11;
	break;

	case 0x35:
		//35		DEC (HL)		11	3	1
		n = memory->Read8(hl);
		ExecDEC(&n);
		memory->Write8(hl, n);
		optime += 11;
	break;

	case 0x36:
		//36 n		LD (HL),n		10	3	1
		n = ReadNext8();
		ExecLD(&t, n);
		memory->Write8(hl, t);
		optime += 10;
	break;

	case 0x37:
		//37		SCF			4	1	1
		ExecSCF();
		optime += 4;
	break;

	case 0x38:
		//3B		DEC SP			6	1	1
		ExecDEC(&sp);
		optime += 6;
	break;

	case 0x39:
		//39		ADD HL,SP		11	3	1
		ExecADD(&hl, sp);
		optime += 11;
	break;

	case 0x3a:
		//3A      LD   A,(nn)     LD  A,(HL-)
		n = memory->Read8(hl);
		ExecLD(&a, n);
		ExecDEC(&hl);
		optime += 9;	//?? based on 0x2a.
	break;

	case 0x3b:
		//3B		DEC SP			6	1	1
		ExecDEC(&sp);
		optime += 6;
	break;

	case 0x3c:
		//3C		INC A			4	1	1
		ExecINC(&a);
		optime += 4;
	break;

	case 0x3d:
		//3D		DEC A			4	1	1
		ExecDEC(&a);
		optime += 4;
	break;

	case 0x3e:
		//3E n		LD A,n			7	2	1
		n = ReadNext8();
		ExecLD(&a, n);
		optime += 7;
	break;

	case 0x3f:
		//3F		CCF			4	1	1
		ExecCCF();
		optime += 4;
	break;


	// 4x


	case 0x40:
		//40		LD B,B			4	1	1
		ExecLD(&b, b);
		optime += 4;
	break;

	case 0x41:
		//41		LD B,C			4	1	1
		ExecLD(&b, c);
		optime += 4;
	break;

	case 0x42:
		//42		LD B,D			4	1	1
		ExecLD(&b, d);
		optime += 4;
	break;

	case 0x43:
		//43		LD B,E			4	1	1
		ExecLD(&b, e);
		optime += 4;
	break;

	case 0x44:
		//44		LD B,H			4	1	1
		ExecLD(&b, h);
		optime += 4;
	break;

	case 0x45:
		//45		LD B,L			4	1	1
		ExecLD(&b, l);
		optime += 4;
	break;

	case 0x46:
		//46		LD B,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecLD(&b, n);
		optime += 7;
	break;

	case 0x47:
		//47		LD B,A			4	1	1
		ExecLD(&b, a);
		optime += 4;
	break;

	case 0x48:
		//48		LD C,B			4	1	1
		ExecLD(&c, b);
		optime += 4;
	break;

	case 0x49:
		//49		LD C,C			4	1	1
		ExecLD(&c, c);
		optime += 4;
	break;

	case 0x4a:
		//4A		LD C,D			4	1	1
		ExecLD(&c, d);
		optime += 4;
	break;

	case 0x4b:
		//4B		LD C,E			4	1	1
		ExecLD(&c, e);
		optime += 4;
	break;

	case 0x4c:
		//4C		LD C,H			4	1	1
		ExecLD(&c, h);
		optime += 4;
	break;

	case 0x4d:
		//4D		LD C,L			4	1	1
		ExecLD(&c, l);
		optime += 4;
	break;

	case 0x4e:
		//4E		LD C,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecLD(&c, n);
		optime += 7;
	break;

	case 0x4f:
		//4F		LD C,A			4	1	1
		ExecLD(&c, a);
		optime += 4;
	break;


	// 5x


	case 0x50:
		//50		LD D,B			4	1	1
		ExecLD(&d, b);
		optime += 4;
	break;

	case 0x51:
		//51		LD D,C			4	1	1
		ExecLD(&d, c);
		optime += 4;
	break;

	case 0x52:
		//52		LD D,D			4	1	1
		ExecLD(&d, d);
		optime += 4;
	break;

	case 0x53:
		//53		LD D,E			4	1	1
		ExecLD(&d, e);
		optime += 4;
	break;

	case 0x54:
		//54		LD D,H			4	1	1
		ExecLD(&d, h);
		optime += 4;
	break;

	case 0x55:
		//55		LD D,L			4	1	1
		ExecLD(&d, l);
		optime += 4;
	break;

	case 0x56:
		//56		LD D,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecLD(&d, n);
		optime += 7;
	break;

	case 0x57:
		//57		LD D,A			4	1	1
		ExecLD(&d, a);
		optime += 4;
	break;

	case 0x58:
		//58		LD E,B			4	1	1
		ExecLD(&e, b);
		optime += 4;
	break;

	case 0x59:
		//59		LD E,C			4	1	1
		ExecLD(&e, c);
		optime += 4;
	break;

	case 0x5a:
		//5A		LD E,D			4	1	1
		ExecLD(&e, d);
		optime += 4;
	break;

	case 0x5b:
		//5B		LD E,E			4	1	1
		ExecLD(&e, e);
		optime += 4;
	break;

	case 0x5c:
		//5C		LD E,H			4	1	1
		ExecLD(&e, h);
		optime += 4;
	break;

	case 0x5d:
		//5D		LD E,L			4	1	1
		ExecLD(&e, l);
		optime += 4;
	break;

	case 0x5e:
		//5E		LD E,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecLD(&e, n);
		optime += 7;
	break;

	case 0x5f:
		//5F		LD E,A			4	1	1
		ExecLD(&e, a);
		optime += 4;
	break;


	// 6x


	case 0x60:
		//60		LD H,B			4	1	1
		ExecLD(&h, b);
		optime += 4;
	break;

	case 0x61:
		//61		LD H,C			4	1	1
		ExecLD(&h, c);
		optime += 4;
	break;

	case 0x62:
		//62		LD H,D			4	1	1
		ExecLD(&h, d);
		optime += 4;
	break;

	case 0x63:
		//63		LD H,E			4	1	1
		ExecLD(&h, e);
		optime += 4;
	break;

	case 0x64:
		//64		LD H,H			4	1	1
		ExecLD(&h, h);
		optime += 4;
	break;

	case 0x65:
		//65		LD H,L			4	1	1
		ExecLD(&h, l);
		optime += 4;
	break;

	case 0x66:
		//66		LD H,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecLD(&h, n);
		optime += 7;
	break;

	case 0x67:
		//67		LD H,A			4	1	1
		ExecLD(&h, a);
		optime += 4;
	break;

	case 0x68:
		//68		LD L,B			4	1	1
		ExecLD(&l, b);
		optime += 4;
	break;

	case 0x69:
		//69		LD L,C			4	1	1
		ExecLD(&l, c);
		optime += 4;
	break;

	case 0x6a:
		//6A		LD L,D			4	1	1
		ExecLD(&l, d);
		optime += 4;
	break;

	case 0x6b:
		//6B		LD L,E			4	1	1
		ExecLD(&l, e);
		optime += 4;
	break;

	case 0x6c:
		//6C		LD L,H			4	1	1
		ExecLD(&l, h);
		optime += 4;
	break;

	case 0x6d:
		//6D		LD L,L			4	1	1
		ExecLD(&l, l);
		optime += 4;
	break;

	case 0x6e:
		//6E		LD L,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecLD(&l, n);
		optime += 7;
	break;

	case 0x6f:
		//6F		LD L,A			4	1	1
		ExecLD(&l, a);
		optime += 4;
	break;


	// 7x


	case 0x70:
		//70		LD (HL),B		7	2	1
		ExecLD(&n, b);
		memory->Write8(hl, n);
		optime += 7;
	break;

	case 0x71:
		//71		LD (HL),C		7	2	1
		ExecLD(&n, c);
		memory->Write8(hl, n);
		optime += 7;
	break;

	case 0x72:
		//72		LD (HL),D		7	2	1
		ExecLD(&n, d);
		memory->Write8(hl, n);
		optime += 7;
	break;

	case 0x73:
		//73		LD (HL),E		7	2	1
		ExecLD(&n, e);
		memory->Write8(hl, n);
		optime += 7;
	break;

	case 0x74:
		//74		LD (HL),H		7	2	1
		ExecLD(&n, h);
		memory->Write8(hl, n);
		optime += 7;
	break;

	case 0x75:
		//75		LD (HL),L		7	2	1
		ExecLD(&n, l);
		memory->Write8(hl, n);
		optime += 7;
	break;

	case 0x76:
		//76		HALT			4	1	1	(repeated till next int)
		halted = true;
		optime += 4;
	break;

	case 0x77:
		//77		LD (HL),A		7	2	1
		ExecLD(&n, a);
		memory->Write8(hl, n);
		optime += 7;
	break;

	case 0x78:
		//78		LD A,B			4	1	1
		ExecLD(&a, b);
		optime += 4;
	break;

	case 0x79:
		//79		LD A,C			4	1	1
		ExecLD(&a, c);
		optime += 4;
	break;

	case 0x7a:
		//7A		LD A,D			4	1	1
		ExecLD(&a, d);
		optime += 4;
	break;

	case 0x7b:
		//7B		LD A,E			4	1	1
		ExecLD(&a, e);
		optime += 4;
	break;

	case 0x7c:
		//7C		LD A,H			4	1	1
		ExecLD(&a, h);
		optime += 4;
	break;

	case 0x7d:
		//7D		LD A,L			4	1	1
		ExecLD(&a, l);
		optime += 4;
	break;

	case 0x7e:
		//7E		LD A,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecLD(&a, n);
		optime += 7;
	break;

	case 0x7f:
		//7F		LD A,A			4	1	1
		ExecLD(&a, a);
		optime += 4;
	break;


	// 8x


	case 0x80:
		//80		ADD A,B			4	1	1
		ExecADD(&a, b);
		optime += 4;
	break;

	case 0x81:
		//81		ADD A,C			4	1	1
		ExecADD(&a, c);
		optime += 4;
	break;

	case 0x82:
		//82		ADD A,D			4	1	1
		ExecADD(&a, d);
		optime += 4;
	break;

	case 0x83:
		//83		ADD A,E			4	1	1
		ExecADD(&a, e);
		optime += 4;
	break;

	case 0x84:
		//84		ADD A,H			4	1	1
		ExecADD(&a, h);
		optime += 4;
	break;

	case 0x85:
		//85		ADD A,L			4	1	1
		ExecADD(&a, l);
		optime += 4;
	break;

	case 0x86:
		//86		ADD A,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecADD(&a, n);
		optime += 7;
	break;

	case 0x87:
		//87		ADD A,A			4	1	1
		ExecADD(&a, a);
		optime += 4;
	break;

	case 0x88:
		//88		ADC A,B			4	1	1
		ExecADC(&a, b);
		optime += 4;
	break;

	case 0x89:
		//89		ADC A,C			4	1	1
		ExecADC(&a, c);
		optime += 4;
	break;

	case 0x8a:
		//8A		ADC A,D			4	1	1
		ExecADC(&a, d);
		optime += 4;
	break;

	case 0x8b:
		//8B		ADC A,E			4	1	1
		ExecADC(&a, e);
		optime += 4;
	break;

	case 0x8c:
		//8C		ADC A,H			4	1	1
		ExecADC(&a, h);
		optime += 4;
	break;

	case 0x8d:
		//8D		ADC A,L			4	1	1
		ExecADC(&a, l);
		optime += 4;
	break;

	case 0x8e:
		//8E		ADC A,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecADC(&a, n);
		optime += 7;
	break;

	case 0x8f:
		//8F		ADC A,A			4	1	1
		ExecADC(&a, a);
		optime += 4;
	break;


	// 9x


	case 0x90:
		//90		SUB B			4	1	1
		ExecSUB(b);
		optime += 4;
	break;

	case 0x91:
		//91		SUB C			4	1	1
		ExecSUB(c);
		optime += 4;
	break;

	case 0x92:
		//92		SUB D			4	1	1
		ExecSUB(d);
		optime += 4;
	break;

	case 0x93:
		//93		SUB E			4	1	1
		ExecSUB(e);
		optime += 4;
	break;

	case 0x94:
		//94		SUB H			4	1	1
		ExecSUB(h);
		optime += 4;
	break;

	case 0x95:
		//95		SUB L			4	1	1
		ExecSUB(l);
		optime += 4;
	break;

	case 0x96:
		//96		SUB (HL)		7	2	1
		n = memory->Read8(hl);
		ExecSUB(n);
		optime += 7;
	break;

	case 0x97:
		//97		SUB A			4	1	1
		ExecSUB(a);
		optime += 4;
	break;

	case 0x98:
		//98		SBC A,B			4	1	1
		ExecSBC(&a, b);
		optime += 4;
	break;

	case 0x99:
		//99		SBC A,C			4	1	1
		ExecSBC(&a, c);
		optime += 4;
	break;

	case 0x9a:
		//9A		SBC A,D			4	1	1
		ExecSBC(&a, d);
		optime += 4;
	break;

	case 0x9b:
		//9B		SBC A,E			4	1	1
		ExecSBC(&a, e);
		optime += 4;
	break;

	case 0x9c:
		//9C		SBC A,H			4	1	1
		ExecSBC(&a, h);
		optime += 4;
	break;

	case 0x9d:
		//9D		SBC A,L			4	1	1
		ExecSBC(&a, l);
		optime += 4;
	break;

	case 0x9e:
		//9E		SBC A,(HL)		7	2	1
		n = memory->Read8(hl);
		ExecSBC(&a, n);
		optime += 7;
	break;

	case 0x9f:
		//9F		SBC A,A			4	1	1
		ExecSBC(&a, a);
		optime += 4;
	break;


	// ax


	case 0xa0:
		//A0		AND B			4	1	1
		ExecAND(b);
		optime += 4;
	break;

	case 0xa1:
		//A1		AND C			4	1	1
		ExecAND(c);
		optime += 4;
	break;

	case 0xa2:
		//A2		AND D			4	1	1
		ExecAND(d);
		optime += 4;
	break;

	case 0xa3:
		//A3		AND E			4	1	1
		ExecAND(e);
		optime += 4;
	break;

	case 0xa4:
		//A4		AND H			4	1	1
		ExecAND(h);
		optime += 4;
	break;

	case 0xa5:
		//A5		AND L			4	1	1
		ExecAND(l);
		optime += 4;
	break;

	case 0xa6:
		//A6		AND (HL)		7	2	1
		n = memory->Read8(hl);
		ExecAND(n);
		optime += 7;
	break;

	case 0xa7:
		//A7		AND A			4	1	1
		ExecAND(a);
		optime += 4;
	break;

	case 0xa8:
		//A8		XOR B			4	1	1
		ExecXOR(b);
		optime += 4;
	break;

	case 0xa9:
		//A9		XOR C			4	1	1
		ExecXOR(c);
		optime += 4;
	break;

	case 0xaa:
		//AA		XOR D			4	1	1
		ExecXOR(d);
		optime += 4;
	break;

	case 0xab:
		//AB		XOR E			4	1	1
		ExecXOR(e);
		optime += 4;
	break;

	case 0xac:
		//AC		XOR H			4	1	1
		ExecXOR(h);
		optime += 4;
	break;

	case 0xad:
		//AD		XOR L			4	1	1
		ExecXOR(l);
		optime += 4;
	break;

	case 0xae:
		//AE		XOR (HL)		7	2	1
		n = memory->Read8(hl);
		ExecXOR(n);
		optime += 7;
	break;

	case 0xaf:
		//AF		XOR A			4	1	1
		ExecAND(b);
		optime += 4;
	break;


	// bx


	case 0xb0:
		//B0		OR B			4	1	1
		ExecOR(b);
		optime += 4;
	break;

	case 0xb1:
		//B1		OR C			4	1	1
		ExecOR(c);
		optime += 4;
	break;

	case 0xb2:
		//B2		OR D			4	1	1
		ExecOR(d);
		optime += 4;
	break;

	case 0xb3:
		//B3		OR E			4	1	1
		ExecOR(e);
		optime += 4;
	break;

	case 0xb4:
		//B4		OR H			4	1	1
		ExecOR(h);
		optime += 4;
	break;

	case 0xb5:
		//B5		OR L			4	1	1
		ExecOR(l);
		optime += 4;
	break;

	case 0xb6:
		//B6		OR (HL)			7	2	1
		n = memory->Read8(hl);
		ExecOR(n);
		optime += 7;
	break;

	case 0xb7:
		//B7		OR A			4	1	1
		ExecOR(a);
		optime += 4;
	break;

	case 0xb8:
		//B8		CP B			4	1	1
		ExecCP(b);
		optime += 4;
	break;

	case 0xb9:
		//B9		CP C			4	1	1
		ExecCP(c);
		optime += 4;
	break;

	case 0xba:
		//BA		CP D			4	1	1
		ExecCP(d);
		optime += 4;
	break;

	case 0xbb:
		//BB		CP E			4	1	1
		ExecCP(e);
		optime += 4;
	break;

	case 0xbc:
		//BC		CP H			4	1	1
		ExecCP(h);
		optime += 4;
	break;

	case 0xbd:
		//BD		CP L			4	1	1
		ExecCP(l);
		optime += 4;
	break;

	case 0xbe:
		//BE		CP (HL)			7	2	1
		n = memory->Read8(hl);
		ExecCP(n);
		optime += 7;
	break;

	case 0xbf:
		//BF		CP A			4	1	1
		ExecCP(a);
		optime += 4;
	break;


	// cx


	case 0xc0:
	break;

	case 0xc1:
	break;

	case 0xc2:
	break;

	case 0xc3:
	break;

	case 0xc4:
	break;

	case 0xc5:
	break;

	case 0xc6:
	break;

	case 0xc7:
	break;

	case 0xc8:
	break;

	case 0xc9:
	break;

	case 0xca:
	break;

	case 0xcb:
	break;

	case 0xcc:
	break;

	case 0xcd:
	break;

	case 0xce:
	break;

	case 0xcf:
	break;


	// dx


	case 0xd0:
	break;

	case 0xd1:
	break;

	case 0xd2:
	break;

	case 0xd3:
	break;

	case 0xd4:
	break;

	case 0xd5:
	break;

	case 0xd6:
	break;

	case 0xd7:
	break;

	case 0xd8:
	break;

	case 0xd9:
	break;

	case 0xda:
	break;

	case 0xdb:
	break;

	case 0xdc:
	break;

	case 0xdd:
	break;

	case 0xde:
	break;

	case 0xdf:
	break;


	// ex


	case 0xe0:
	break;

	case 0xe1:
	break;

	case 0xe2:
	break;

	case 0xe3:
	break;

	case 0xe4:
	break;

	case 0xe5:
	break;

	case 0xe6:
	break;

	case 0xe7:
	break;

	case 0xe8:
	break;

	case 0xe9:
	break;

	case 0xea:
	break;

	case 0xeb:
	break;

	case 0xec:
	break;

	case 0xed:
	break;

	case 0xee:
	break;

	case 0xef:
	break;


	// fx


	case 0xf0:
	break;

	case 0xf1:
	break;

	case 0xf2:
	break;

	case 0xf3:
	break;

	case 0xf4:
	break;

	case 0xf5:
	break;

	case 0xf6:
	break;

	case 0xf7:
	break;

	case 0xf8:
	break;

	case 0xf9:
	break;

	case 0xfa:
	break;

	case 0xfb:
	break;

	case 0xfc:
	break;

	case 0xfd:
	break;

	case 0xfe:
	break;

	case 0xff:
	break;



	default:
	break;
	}


	return optime;
}

int CPU::ExecuteCB()
{
	u8 opcode = memory->Read8(pc++);
	return 0;
}

/*
Moved, Removed, and Added Opcodes
Opcode  Z80             GMB
---------------------------------------
08      EX   AF,AF      LD   (nn),SP
10      DJNZ PC+dd      STOP
22      LD   (nn),HL    LDI  (HL),A
2A      LD   HL,(nn)    LDI  A,(HL)
32      LD   (nn),A     LDD  (HL),A
3A      LD   A,(nn)     LDD  A,(HL)
D3      OUT  (n),A      -
D9      EXX             RETI
DB      IN   A,(n)      -
DD      <IX>            -
E0      RET  PO         LD   (FF00+n),A
E2      JP   PO,nn      LD   (FF00+C),A
E3      EX   (SP),HL    -
E4      CALL P0,nn      -
E8      RET  PE         ADD  SP,dd
EA      JP   PE,nn      LD   (nn),A
EB      EX   DE,HL      -
EC      CALL PE,nn      -
ED      <pref>          -
F0      RET  P          LD   A,(FF00+n)
F2      JP   P,nn       LD   A,(FF00+C)
F4      CALL P,nn       -
F8      RET  M          LD   HL,SP+dd
FA      JP   M,nn       LD   A,(nn)
FC      CALL M,nn       -
FD      <IY>            -
CB3X    SLL  r/(HL)     SWAP r/(HL)
Note: The unused (-) opcodes will lock-up the gameboy CPU when used.
*/

/*
00		NOP			4	1	1
01 n n		LD BC,nn		10	3	1
02		LD (BC),A		7	2	1
03		INC BC			6	1	1
04		INC B			4	1	1
05		DEC B			4	1	1
06 n		LD B,n			7	2	1
07		RLCA			4	1	1
08		EX AF,AF’		4	1	1
09		ADD HL,BC		11	3	1
0A		LD A,(BC)		7	2	1
0B		DEC BC			6	1	1
0C		INC C			4	1	1
0D		DEC C			4	1	1
0E n		LD C,n			7	2	1
0F		RRCA			4	1	1
10 e		DJNZ (PC+e)		8/13	2/3	1/1	(met/not met)
11 n n		LD DE,nn		10	3	1
12		LD (DE),A		7	2	1
13		INC DE			6	1	1
14		INC D			4	1	1
15		DEC D			4	1	1
16 n		LD D,n			7	2	1
17		RLA			4	1	1
18 e		JR (PC+e)		12	3	1
19		ADD HL,DE		11	3	1
1A		LD A,(DE)		7	2	1
1B		DEC DE			6	1	1
1C		INC E			4	1	1
1D		DEC E			4	1	1
1E n		LD E,n			7	2	1
1F		RRA			4	1	1
20 e		JR NZ,(PC+e)		12/7	3/2	1/1	(met/not met)
21 n n		LD HL,nn		10	3	1
22 n n		LD (nn),HL		16	5	3
23		INC HL			6	1	1
24		INC H			4	1	1
25		DEC H			4	1	1
26 n		LD H,n			7	2	1
27		DAA			4	1	1
28 e		JR Z,(PC+e)		12/7	3/2	1/1	(met/not met)
29		ADD HL,HL		11	3	1
2A n n		LD HL,(nn)		16	5	1
2B		DEC HL			6	1	1
2C		INC L			4	1	1
2D		DEC L			4	1	1
2E n		LD L,n			7	2	1
2F		CPL			4	1	1
30 e		JR NC,(PC+e)		12/7	3/2	1/1	(met/not met)
31 n n		LD SP,nn		10	3	1
32 n n		LD (nn),A		13	4	1
33		INC SP			6	1	1
34		INC (HL)		11	3	1
35		DEC (HL)		11	3	1
36 n		LD (HL),n		10	3	1
37		SCF			4	1	1
38 e		JR C,(PC+e)		12/7	3/2	1/1	(met/not met)
39		ADD HL,SP		11	3	1
3A n n		LD A,(nn)		13	4	1
3B		DEC SP			6	1	1
3C		INC A			4	1	1
3D		DEC A			4	1	1
3E n		LD A,n			7	2	1
3F		CCF			4	1	1
40		LD B,B			4	1	1
41		LD B,C			4	1	1
42		LD B,D			4	1	1
43		LD B,E			4	1	1
44		LD B,H			4	1	1
45		LD B,L			4	1	1
46		LD B,(HL)		7	2	1
47		LD B,A			4	1	1
48		LD C,B			4	1	1
49		LD C,C			4	1	1
4A		LD C,D			4	1	1
4B		LD C,E			4	1	1
4C		LD C,H			4	1	1
4D		LD C,L			4	1	1
4E		LD C,(HL)		7	2	1
4F		LD C,A			4	1	1
50		LD D,B			4	1	1
51		LD D,C			4	1	1
52		LD D,D			4	1	1
53		LD D,E			4	1	1
54		LD D,H			4	1	1
55		LD D,L			4	1	1
56		LD D,(HL)		7	2	1
57		LD D,A			4	1	1
58		LD E,B			4	1	1
59		LD E,C			4	1	1
5A		LD E,D			4	1	1
5B		LD E,E			4	1	1
5C		LD E,H			4	1	1
5D		LD E,L			4	1	1
5E		LD E,(HL)		7	2	1
5F		LD E,A			4	1	1
60		LD H,B			4	1	1
61		LD H,C			4	1	1
62		LD H,D			4	1	1
63		LD H,E			4	1	1
64		LD H,H			4	1	1
65		LD H,L			4	1	1
66		LD H,(HL)		7	2	1
67		LD H,A			4	1	1
68		LD L,B			4	1	1
69		LD L,C			4	1	1
6A		LD L,D			4	1	1
6B		LD L,E			4	1	1
6C		LD L,H			4	1	1
6D		LD L,L			4	1	1
6E		LD L,(HL)		7	2	1
6F		LD L,A			4	1	1
70		LD (HL),B		7	2	1
71		LD (HL),C		7	2	1
72		LD (HL),D		7	2	1
73		LD (HL),E		7	2	1
74		LD (HL),H		7	2	1
75		LD (HL),L		7	2	1
76		HALT			4	1	1	(repeated till next int)
77		LD (HL),A		7	2	1
78		LD A,B			4	1	1
79		LD A,C			4	1	1
7A		LD A,D			4	1	1
7B		LD A,E			4	1	1
7C		LD A,H			4	1	1
7D		LD A,L			4	1	1
7E		LD A,(HL)		7	2	1
7F		LD A,A			4	1	1
80		ADD A,B			4	1	1
81		ADD A,C			4	1	1
82		ADD A,D			4	1	1
83		ADD A,E			4	1	1
84		ADD A,H			4	1	1
85		ADD A,L			4	1	1
86		ADD A,(HL)		7	2	1
87		ADD A,A			4	1	1
88		ADC A,B			4	1	1
89		ADC A,C			4	1	1
8A		ADC A,D			4	1	1
8B		ADC A,E			4	1	1
8C		ADC A,H			4	1	1
8D		ADC A,L			4	1	1
8E		ADC A,(HL)		7	2	1
8F		ADC A,A			4	1	1
90		SUB B			4	1	1
91		SUB C			4	1	1
92		SUB D			4	1	1
93		SUB E			4	1	1
94		SUB H			4	1	1
95		SUB L			4	1	1
96		SUB (HL)		7	2	1
97		SUB A			4	1	1
98		SBC A,B			4	1	1
99		SBC A,C			4	1	1
9A		SBC A,D			4	1	1
9B		SBC A,E			4	1	1
9C		SBC A,H			4	1	1
9D		SBC A,L			4	1	1
9E		SBC A,(HL)		7	2	1
9F		SBC A,A			4	1	1
A0		AND B			4	1	1
A1		AND C			4	1	1
A2		AND D			4	1	1
A3		AND E			4	1	1
A4		AND H			4	1	1
A5		AND L			4	1	1
A6		AND (HL)		7	2	1
A7		AND A			4	1	1
A8		XOR B			4	1	1
A9		XOR C			4	1	1
AA		XOR D			4	1	1
AB		XOR E			4	1	1
AC		XOR H			4	1	1
AD		XOR L			4	1	1
AE		XOR (HL)		7	2	1
AF		XOR A			4	1	1
B0		OR B			4	1	1
B1		OR C			4	1	1
B2		OR D			4	1	1
B3		OR E			4	1	1
B4		OR H			4	1	1
B5		OR L			4	1	1
B6		OR (HL)			7	2	1
B7		OR A			4	1	1
B8		CP B			4	1	1
B9		CP C			4	1	1
BA		CP D			4	1	1
BB		CP E			4	1	1
BC		CP H			4	1	1
BD		CP L			4	1	1
BE		CP (HL)			7	2	1
BF		CP A			4	1	1
C0		RET NZ			11/5	3/1	1/1	(met/not met)
C1		POP BC			10	3	1
C2 n n		JP NZ,(nn)		10	3	1	(met or not)
C3 n n		JP (nn)			10	3	1
C4 n n		CALL NZ,(nn)		17/10	5/3	1/1	(met/not met)
C5		PUSH BC			11	3	1
C6 n		ADD A,n			7	2	1
C7		RST 0H			11	3	1
C8		RET Z			11/5	3/1	1/1	(met/not met)
C9		RET			10	3	1
CA n n		JP Z,(nn)		10	3	1	(always same)
CC n n		CALL Z,(nn)		17/10	5/3	1/1	(met/not met)
CD n n		CALL (nn)		17	5	1
CE n		ADC A,n			7	2	1
CF		RST 8H			11	3	1
D0		RET NC			5	1	1
D1		POP DE			10	3	1
D2 n n		JP NC,(nn)		10	3	1	(met or not)
D3 n		OUT (n),A		11	3	1
D4 n n		CALL NC,(nn)		17/10	5/3	1/1	(met/not met)
D5		PUSH DE			11	3	1
D6 n		SUB n			7	2	1
D7		RST 10H			11	3	1
D8		RET C			5	1	1
D9		EXX			4	1	1
DA n n		JP C,(nn)		10	3	1	(met or not)
DB n		IN A,(n)		11	3	1
DC n n		CALL C,(nn)		17/10	5/3	1
DE n		SBC A,n
DF		RST 18H
E0		RET PO
E1		POP HL
E2 n n		JP PO,(nn)
E3		EX (SP),HL
E4 n n		CALL PO,(nn)
E5		PUSH HL
E6 n		AND n
E7		RST 20H
E8		RET PE
E9		JP (HL)
EA n n		JP PE,(nn)
EB		EX DE,HL
EC n n		CALL PE,(nn)
EE n		XOR n
EF		RST 28H
F0		RET P
F1		POP AF
F2 n n		JP P,(nn)
F3		DI
F4 n n		CALL P,(nn)
F5		PUSH AF
F6 n		OR n
F7		RST 30H
F8		RET M
F9		LD SP,HL
FA n n		JP M,(nn)
FB		EI
FC n n		CALL M,(nn)
FE n		CP n
FF		RST 38H
*/

/*
CB00		RLC B			8	2	2
CB01		RLC C			8	2	2
CB02		RLC D			8	2	2
CB03		RLC E			8	2	2
CB04		RLC H			8	2	2
CB05		RLC L			8	2	2
CB06		RLC (HL)		15	4	2
CB07		RLC A			8	2	2
CB08		RRC B			8	2	2
CB09		RRC C			8	2	2
CB0A		RRC D			8	2	2
CB0B		RRC E			8	2	2
CB0C		RRC H			8	2	2
CB0D		RRC L			8	2	2
CB0E		RRC (HL)		15	4	2
CB0F		RRC A			8	2	2
CB10		RL B			8	2	2
CB11		RL C			8	2	2
CB12		RL D			8	2	2
CB13		RL E			8	2	2
CB14		RL H			8	2	2
CB15		RL L			8	2	2
CB16		RL (HL)			15	4	2
CB17		RL A			8	2	2
CB18		RR B			8	2	2
CB19		RR C			8	2	2
CB1A		RR D			8	2	2
CB1B		RR E			8	2	2
CB1C		RR H			8	2	2
CB1D		RR L			8	2	2
CB1E		RR (HL)			15	4	2
CB1F		RR A			8	2	2
CB20		SLA B			8	2	2
CB21		SLA C			8	2	2
CB22		SLA D			8	2	2
CB23		SLA E			8	2	2
CB24		SLA H			8	2	2
CB25		SLA L			8	2	2
CB26		SLA (HL)		15	4	2
CB27		SLA A			8	2	2
CB28		SRA B			8	2	2
CB29		SRA C			8	2	2
CB2A		SRA D			8	2	2
CB2B		SRA E			8	2	2
CB2C		SRA H			8	2	2
CB2D		SRA L			8	2	2
CB2E		SRA (HL)		15	4	2
CB2F		SRA A			8	2	2
CB30		SLL B*			8	2	2
CB31		SLL C*			8	2	2
CB32		SLL D*			8	2	2
CB33		SLL E*			8	2	2
CB34		SLL H*			8	2	2
CB35		SLL L*			8	2	2
CB36		SLL (HL)*		15	4	2
CB37		SLL A*			8	2	2
CB38		SRL B			8	2	2
CB39		SRL C			8	2	2
CB3A		SRL D			8	2	2
CB3B		SRL E			8	2	2
CB3C		SRL H			8	2	2
CB3D		SRL L			8	2	2
CB3E		SRL (HL)		15	4	2
CB3F		SRL A			8	2	2
CB40		BIT 0,B			8	2	2
CB41		BIT 0,C			8	2	2
CB42		BIT 0,D			8	2	2
CB43		BIT 0,E			8	2	2
CB44	 	BIT 0,H			8	2	2
CB45	 	BIT 0,L			8	2	2
CB46	 	BIT 0,(HL)		12	3	2
CB47	 	BIT 0,A			8	2	2
CB48		BIT 1,B			8	2	2
CB49	 	BIT 1,C			8	2	2
CB4A	 	BIT 1,D			8	2	2
CB4B	 	BIT 1,E			8	2	2
CB4C	 	BIT 1,H			8	2	2
CB4D	 	BIT 1,L			8	2	2
CB4E	 	BIT 1,(HL)		12	3	2
CB4F	 	BIT 1,A			8	2	2
CB50		BIT 2,B			8	2	2
CB51	 	BIT 2,C			8	2	2
CB52	 	BIT 2,D			8	2	2
CB53	 	BIT 2,E			8	2	2
CB54	 	BIT 2,H			8	2	2
CB55	 	BIT 2,L			8	2	2
CB56	 	BIT 2,(HL)		12	3	2
CB57	 	BIT 2,A			8	2	2
CB58		BIT 3,B			8	2	2
CB59	 	BIT 3,C			8	2	2
CB5A	 	BIT 3,D			8	2	2
CB5B	 	BIT 3,E			8	2	2
CB5C	 	BIT 3,H			8	2	2
CB5D	 	BIT 3,L			8	2	2
CB5E	 	BIT 3,(HL)		12	3	2
CB5F	 	BIT 3,A			8	2	2
CB60		BIT 4,B			8	2	2
CB61	 	BIT 4,C			8	2	2
CB62	 	BIT 4,D			8	2	2
CB63	 	BIT 4,E			8	2	2
CB64	 	BIT 4,H			8	2	2
CB65	 	BIT 4,L			8	2	2
CB66	 	BIT 4,(HL)		12	3	2
CB67	 	BIT 4,A			8	2	2
CB68		BIT 5,B			8	2	2
CB69	 	BIT 5,C			8	2	2
CB6A	 	BIT 5,D			8	2	2
CB6B	 	BIT 5,E			8	2	2
CB6C	 	BIT 5,H			8	2	2
CB6D	 	BIT 5,L			8	2	2
CB6E	 	BIT 5,(HL)		12	3	2
CB6F	 	BIT 5,A			8	2	2
CB70		BIT 6,B			8	2	2
CB71	 	BIT 6,C			8	2	2
CB72	 	BIT 6,D			8	2	2
CB73	 	BIT 6,E			8	2	2
CB74	 	BIT 6,H			8	2	2
CB75	 	BIT 6,L			8	2	2
CB76	 	BIT 6,(HL)		12	3	2
CB77	 	BIT 6,A			8	2	2
CB78		BIT 7,B			8	2	2
CB79	 	BIT 7,C			8	2	2
CB7A	 	BIT 7,D			8	2	2
CB7B	 	BIT 7,E			8	2	2
CB7C	 	BIT 7,H			8	2	2
CB7D	 	BIT 7,L			8	2	2
CB7E	 	BIT 7,(HL)		12	3	2
CB7F	 	BIT 7,A			8	2	2
CB80		RES 0,B			8	2	2
CB81		RES 0,C			8	2	2
CB82		RES 0,D			8	2	2
CB83		RES 0,E			8	2	2
CB84	 	RES 0,H			8	2	2
CB85	 	RES 0,L			8	2	2
CB86	 	RES 0,(HL)		15	4	2
CB87	 	RES 0,A			8	2	2
CB88		RES 1,B			8	2	2
CB89	 	RES 1,C			8	2	2
CB8A	 	RES 1,D			8	2	2
CB8B	 	RES 1,E			8	2	2
CB8C	 	RES 1,H			8	2	2
CB8D	 	RES 1,L			8	2	2
CB8E	 	RES 1,(HL)		15	4	2
CB8F	 	RES 1,A			8	2	2
CB90		RES 2,B			8	2	2
CB91	 	RES 2,C			8	2	2
CB92	 	RES 2,D			8	2	2
CB93	 	RES 2,E			8	2	2
CB94	 	RES 2,H			8	2	2
CB95	 	RES 2,L			8	2	2
CB96	 	RES 2,(HL)		15	4	2
CB97	 	RES 2,A			8	2	2
CB98		RES 3,B			8	2	2
CB99	 	RES 3,C			8	2	2
CB9A	 	RES 3,D			8	2	2
CB9B	 	RES 3,E			8	2	2
CB9C	 	RES 3,H			8	2	2
CB9D	 	RES 3,L			8	2	2
CB9E	 	RES 3,(HL)		15	4	2
CB9F	 	RES 3,A			8	2	2
CBA0		RES 4,B			8	2	2
CBA1	 	RES 4,C			8	2	2
CBA2	 	RES 4,D			8	2	2
CBA3	 	RES 4,E			8	2	2
CBA4	 	RES 4,H			8	2	2
CBA5	 	RES 4,L			8	2	2
CBA6	 	RES 4,(HL)		15	4	2
CBA7	 	RES 4,A			8	2	2
CBA8		RES 5,B			8	2	2
CBA9	 	RES 5,C			8	2	2
CBAA	 	RES 5,D			8	2	2
CBAB	 	RES 5,E			8	2	2
CBAC	 	RES 5,H			8	2	2
CBAD	 	RES 5,L			8	2	2
CBAE	 	RES 5,(HL)		15	4	2
CBAF	 	RES 5,A			8	2	2
CBB0		RES 6,B			8	2	2
CBB1	 	RES 6,C			8	2	2
CBB2	 	RES 6,D			8	2	2
CBB3	 	RES 6,E			8	2	2
CBB4	 	RES 6,H			8	2	2
CBB5	 	RES 6,L			8	2	2
CBB6	 	RES 6,(HL)		15	4	2
CBB7	 	RES 6,A			8	2	2
CBB8		RES 7,B			8	2	2
CBB9	 	RES 7,C			8	2	2
CBBA	 	RES 7,D			8	2	2
CBBB	 	RES 7,E			8	2	2
CBBC	 	RES 7,H			8	2	2
CBBD	 	RES 7,L			8	2	2
CBBE	 	RES 7,(HL)		15	4	2
CBBF	 	RES 7,A			8	2	2
CBC0		SET 0,B			8	2	2
CBC1		SET 0,C			8	2	2
CBC2		SET 0,D			8	2	2
CBC3		SET 0,E			8	2	2
CBC4	 	SET 0,H			8	2	2
CBC5	 	SET 0,L			8	2	2
CBC6	 	SET 0,(HL)		15	4	2
CBC7	 	SET 0,A			8	2	2
CBC8		SET 1,B			8	2	2
CBC9	 	SET 1,C			8	2	2
CBCA	 	SET 1,D			8	2	2
CBCB	 	SET 1,E			8	2	2
CBCC	 	SET 1,H			8	2	2
CBCD	 	SET 1,L			8	2	2
CBCE	 	SET 1,(HL)		15	4	2
CBCF	 	SET 1,A			8	2	2
CBD0		SET 2,B			8	2	2
CBD1	 	SET 2,C			8	2	2
CBD2	 	SET 2,D			8	2	2
CBD3	 	SET 2,E			8	2	2
CBD4	 	SET 2,H			8	2	2
CBD5	 	SET 2,L			8	2	2
CBD6	 	SET 2,(HL)		15	4	2
CBD7	 	SET 2,A			8	2	2
CBD8		SET 3,B			8	2	2
CBD9	 	SET 3,C			8	2	2
CBDA	 	SET 3,D			8	2	2
CBDB	 	SET 3,E			8	2	2
CBDC	 	SET 3,H			8	2	2
CBDD	 	SET 3,L			8	2	2
CBDE	 	SET 3,(HL)		15	4	2
CBDF	 	SET 3,A			8	2	2
CBE0		SET 4,B			8	2	2
CBE1	 	SET 4,C			8	2	2
CBE2	 	SET 4,D			8	2	2
CBE3	 	SET 4,E			8	2	2
CBE4	 	SET 4,H			8	2	2
CBE5	 	SET 4,L			8	2	2
CBE6	 	SET 4,(HL)		15	4	2
CBE7	 	SET 4,A			8	2	2
CBE8		SET 5,B			8	2	2
CBE9	 	SET 5,C			8	2	2
CBEA	 	SET 5,D			8	2	2
CBEB	 	SET 5,E			8	2	2
CBEC	 	SET 5,H			8	2	2
CBED	 	SET 5,L			8	2	2
CBEE	 	SET 5,(HL)		15	4	2
CBEF	 	SET 5,A			8	2	2
CBF0		SET 6,B			8	2	2
CBF1	 	SET 6,C			8	2	2
CBF2	 	SET 6,D			8	2	2
CBF3	 	SET 6,E			8	2	2
CBF4	 	SET 6,H			8	2	2
CBF5	 	SET 6,L			8	2	2
CBF6	 	SET 6,(HL)		15	4	2
CBF7	 	SET 6,A			8	2	2
CBF8		SET 7,B			8	2	2
CBF9	 	SET 7,C			8	2	2
CBFA	 	SET 7,D			8	2	2
CBFB	 	SET 7,E			8	2	2
CBFC	 	SET 7,H			8	2	2
CBFD	 	SET 7,L			8	2	2
CBFE	 	SET 7,(HL)		15	4	2
CBFF	 	SET 7,A			8	2	2
*/