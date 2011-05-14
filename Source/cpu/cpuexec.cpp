#include "cpu.h"

#include "../memory/memory.h"


int CPU::Execute()
{
	m_instructionTime = 0;

	u8 n,t;
	u16 nn,tt;
	u16 address;

	u8 opcode;

	bool executeInterrupt = false;
	u8 interruptFlags = m_memory->Read8(REG_IF);
	interruptFlags &= 0x1f;	///<Only bits 0-4 signal valid interrupts.
	if(m_interruptsEnabled && m_delayNextInterrupt == false && interruptFlags != 0)
	{
		u8 interruptEnableFlags = m_memory->Read8(REG_IE);
		
		if( (interruptFlags & IF_VBLANK) && (interruptEnableFlags & IF_VBLANK) )
		{
			executeInterrupt = true;
			m_interruptsEnabled = false;
			ExecCALL(0x0040);
		}
		else if( (interruptFlags & IF_LCDC) && (interruptEnableFlags & IF_LCDC) )
		{
			executeInterrupt = true;
			m_interruptsEnabled = false;
			ExecCALL(0x0048);
		}
		else if( (interruptFlags & IF_TIMER) && (interruptEnableFlags & IF_TIMER) )
		{
			executeInterrupt = true;
			m_interruptsEnabled = false;
			ExecCALL(0x0050);
		}
		else if( (interruptFlags & IF_SERIAL) && (interruptEnableFlags & IF_SERIAL) )
		{
			executeInterrupt = true;
			m_interruptsEnabled = false;
			ExecCALL(0x0058);
		}
		else if( (interruptFlags & IF_INPUT) && (interruptEnableFlags & IF_INPUT) )
		{
			executeInterrupt = true;
			m_interruptsEnabled = false;
			ExecCALL(0x0060);
		}
	}

	//Interrupts are delayed for one instruction after EI.
	if(m_delayNextInterrupt == true)
	{
		m_delayNextInterrupt = false;
	}
	
	if(m_halted && executeInterrupt == false)
	{
		opcode = 0x00;	//When halted, we just execute NOPs until an interrupt
	}
	else
	{
		opcode = m_memory->Read8(pc);
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
		m_instructionTime += 4;
	break;
	
	case 0x01:
		//01 n n		LD BC,nn		10	3	1
		nn = ReadNext16();
		ExecLD(&bc, nn);
		m_instructionTime += 10;
	break;

	case 0x02:
		//02		LD (BC),A		7	2	1
		ExecLD(&t, a);
		m_memory->Write8(bc, t);
		m_instructionTime += 7;
	break;

	case 0x03:
		//03		INC BC			6	1	1
		ExecINC(&bc);
		m_instructionTime += 6;
	break;

	case 0x04:
		//04		INC B			4	1	1
		ExecINC(&b);
		m_instructionTime += 4;
	break;

	case 0x05:
		//05		DEC B			4	1	1
		ExecDEC(&b);
		m_instructionTime += 4;
	break;

	case 0x06:
		//06 n		LD B,n			7	2	1
		n = ReadNext8();
		ExecLD(&b, n);
		m_instructionTime += 7;
	break;

	case 0x07:
		//07		RLCA			4	1	1
		ExecRLCA();
		m_instructionTime += 4;
	break;

	case 0x08:
		//08      EX   AF,AF      LD   (nn),SP
		nn = ReadNext16();
		ExecLD(&tt, sp);
		m_memory->Write16(nn, tt);
		m_instructionTime += 16;	//based on LD (nn),HL
	break;

	case 0x09:
		//09		ADD HL,BC		11	3	1
		ExecADD(&hl, bc);
		m_instructionTime += 11;
	break;

	case 0x0a:
		//0A		LD A,(BC)		7	2	1
		ExecLD(&a, m_memory->Read8(bc));
		m_instructionTime += 7;
	break;

	case 0x0b:
		//0B		DEC BC			6	1	1
		ExecDEC(&bc);
		m_instructionTime += 6;
	break;

	case 0x0c:
		//0C		INC C			4	1	1
		ExecINC(&c);
		m_instructionTime += 4;
	break;

	case 0x0d:
		//0D		DEC C			4	1	1
		ExecDEC(&c);
		m_instructionTime += 4;
	break;

	case 0x0e:
		//0E n		LD C,n			7	2	1
		n = ReadNext8();
		ExecLD(&c, n);
		m_instructionTime += 7;
	break;

	case 0x0f:
		//0F		RRCA			4	1	1
		ExecRRCA();
		m_instructionTime += 4;
	break;


	// 1x


	case 0x10:
		//10      DJNZ PC+dd      STOP
		//?? Todo. Need to stop the screen and cpu until button press.
	break;

	case 0x11:
		//11 n n		LD DE,nn		10	3	1
		nn = ReadNext16();
		ExecLD(&de, nn);
		m_instructionTime += 10;
	break;

	case 0x12:
		//12		LD (DE),A		7	2	1
		ExecLD(&t, a);
		m_memory->Write8(de, t);
		m_instructionTime += 7;
	break;

	case 0x13:
		//13		INC DE			6	1	1
		ExecINC(&de);
		m_instructionTime += 6;
	break;

	case 0x14:
		//14		INC D			4	1	1
		ExecINC(&d);
		m_instructionTime += 4;
	break;

	case 0x15:
		//15		DEC D			4	1	1
		ExecDEC(&d);
		m_instructionTime += 4;
	break;

	case 0x16:
		//16 n		LD D,n			7	2	1
		n = ReadNext8();
		ExecLD(&d, n);
		m_instructionTime += 7;
	break;

	case 0x17:
		//17		RLA			4	1	1
		ExecRLA();
		m_instructionTime += 4;
	break;

	case 0x18:
		//18 e		JR (PC+e)		12	3	1
		n = ReadNext8();
		ExecJR((s8)n);
		m_instructionTime += 12;
	break;

	case 0x19:
		//19		ADD HL,DE		11	3	1
		ExecADD(&hl, de);
		m_instructionTime += 11;
	break;

	case 0x1a:
		//1A		LD A,(DE)		7	2	1
		n = m_memory->Read8(de);
		ExecLD(&a, n);
		m_instructionTime += 7;
	break;

	case 0x1b:
		//1B		DEC DE			6	1	1
		ExecDEC(&de);
		m_instructionTime += 6;
	break;

	case 0x1c:
		//1C		INC E			4	1	1
		ExecINC(&e);
		m_instructionTime += 4;
	break;

	case 0x1d:
		//1D		DEC E			4	1	1
		ExecDEC(&e);
		m_instructionTime += 4;
	break;

	case 0x1e:
		//1E n		LD E,n			7	2	1
		n = ReadNext8();
		ExecLD(&e, n);
		m_instructionTime += 7;
	break;

	case 0x1f:
		//1F		RRA			4	1	1
		ExecRRA();
		m_instructionTime += 4;
	break;


	// 2x


	case 0x20:
		//20 e		JR NZ,(PC+e)		12/7	3/2	1/1	(met/not met)
		n = ReadNext8();
		if(TST_Z)
		{
			m_instructionTime += 7;
		}
		else
		{
			ExecJR((s8)n);
			m_instructionTime += 12;
		}
	break;

	case 0x21:
		//21 n n		LD HL,nn		10	3	1
		nn = ReadNext16();
		ExecLD(&hl, nn);
		m_instructionTime += 10;
	break;

	case 0x22:
		//22      LD   (nn),HL    LD  (HL+),A
		ExecLD(&n, a);
		m_memory->Write8(hl, n);
		ExecINC(&hl);
		m_instructionTime += 9; //?? Based on Fetch/Decode = 3, INC HL = 6 (guessing 3+3), and LD (HL), A = (4+3) (guessing 3+1 +3)
	break;

	case 0x23:
		//23		INC HL			6	1	1
		ExecINC(&hl);
		m_instructionTime += 6;
	break;

	case 0x24:
		//24		INC H			4	1	1
		ExecINC(&h);
		m_instructionTime += 4;
	break;

	case 0x25:
		//25		DEC H			4	1	1
		ExecDEC(&h);
		m_instructionTime += 4;
	break;

	case 0x26:
		//26 n		LD H,n			7	2	1
		n = ReadNext8();
		ExecLD(&h, n);
		m_instructionTime += 7;
	break;

	case 0x27:
		//27		DAA			4	1	1
		ExecDAA();
		m_instructionTime += 4;
	break;

	case 0x28:
		//28 e		JR Z,(PC+e)		12/7	3/2	1/1	(met/not met)
		n = ReadNext8();
		if(TST_Z)
		{
			ExecJR((s8)n);
			m_instructionTime += 12;
		}
		else
		{
			m_instructionTime += 7;
		}
	break;

	case 0x29:
		//29		ADD HL,HL		11	3	1
		ExecADD(&hl, hl);
		m_instructionTime += 11;
	break;

	case 0x2a:
		//2A      LD   HL,(nn)    LD  A,(HL+)
		n = m_memory->Read8(hl);
		ExecLD(&a, n);
		ExecINC(&hl);
		m_instructionTime += 9;	//?? based on 0x22. check this.
	break;

	case 0x2b:
		//2B		DEC HL			6	1	1
		ExecDEC(&hl);
		m_instructionTime += 6;
	break;

	case 0x2c:
		//2C		INC L			4	1	1
		ExecINC(&l);
		m_instructionTime += 4;
	break;

	case 0x2d:
		//2D		DEC L			4	1	1
		ExecDEC(&l);
		m_instructionTime += 4;
	break;

	case 0x2e:
		//2E n		LD L,n			7	2	1
		n = ReadNext8();
		ExecLD(&l, n);
		m_instructionTime += 7;
	break;

	case 0x2f:
		//2F		CPL			4	1	1
		ExecCPL();
		m_instructionTime += 4;
	break;


	// 3x


	case 0x30:
		//30 e		JR NC,(PC+e)		12/7	3/2	1/1	(met/not met)
		n = ReadNext8();
		if(TST_C)
		{
			m_instructionTime += 7;
		}
		else
		{
			ExecJR((s8)n);
			m_instructionTime += 12;
		}
	break;

	case 0x31:
		//31 n n		LD SP,nn		10	3	1
		nn = ReadNext16();
		ExecLD(&sp, nn);
		m_instructionTime += 10;
	break;

	case 0x32:
		//32      LD   (nn),A     LD  (HL-),A
		ExecLD(&n, a);
		m_memory->Write8(hl, n);
		ExecDEC(&hl);
		m_instructionTime += 9;	//?? based on 0x22.
	break;

	case 0x33:
		//33		INC SP			6	1	1
		ExecINC(&sp);
		m_instructionTime += 6;
	break;

	case 0x34:
		//34		INC (HL)		11	3	1
		n = m_memory->Read8(hl);
		ExecINC(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 11;
	break;

	case 0x35:
		//35		DEC (HL)		11	3	1
		n = m_memory->Read8(hl);
		ExecDEC(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 11;
	break;

	case 0x36:
		//36 n		LD (HL),n		10	3	1
		n = ReadNext8();
		ExecLD(&t, n);
		m_memory->Write8(hl, t);
		m_instructionTime += 10;
	break;

	case 0x37:
		//37		SCF			4	1	1
		ExecSCF();
		m_instructionTime += 4;
	break;

	case 0x38:
		//3B		DEC SP			6	1	1
		ExecDEC(&sp);
		m_instructionTime += 6;
	break;

	case 0x39:
		//39		ADD HL,SP		11	3	1
		ExecADD(&hl, sp);
		m_instructionTime += 11;
	break;

	case 0x3a:
		//3A      LD   A,(nn)     LD  A,(HL-)
		n = m_memory->Read8(hl);
		ExecLD(&a, n);
		ExecDEC(&hl);
		m_instructionTime += 9;	//?? based on 0x2a.
	break;

	case 0x3b:
		//3B		DEC SP			6	1	1
		ExecDEC(&sp);
		m_instructionTime += 6;
	break;

	case 0x3c:
		//3C		INC A			4	1	1
		ExecINC(&a);
		m_instructionTime += 4;
	break;

	case 0x3d:
		//3D		DEC A			4	1	1
		ExecDEC(&a);
		m_instructionTime += 4;
	break;

	case 0x3e:
		//3E n		LD A,n			7	2	1
		n = ReadNext8();
		ExecLD(&a, n);
		m_instructionTime += 7;
	break;

	case 0x3f:
		//3F		CCF			4	1	1
		ExecCCF();
		m_instructionTime += 4;
	break;


	// 4x


	case 0x40:
		//40		LD B,B			4	1	1
		ExecLD(&b, b);
		m_instructionTime += 4;
	break;

	case 0x41:
		//41		LD B,C			4	1	1
		ExecLD(&b, c);
		m_instructionTime += 4;
	break;

	case 0x42:
		//42		LD B,D			4	1	1
		ExecLD(&b, d);
		m_instructionTime += 4;
	break;

	case 0x43:
		//43		LD B,E			4	1	1
		ExecLD(&b, e);
		m_instructionTime += 4;
	break;

	case 0x44:
		//44		LD B,H			4	1	1
		ExecLD(&b, h);
		m_instructionTime += 4;
	break;

	case 0x45:
		//45		LD B,L			4	1	1
		ExecLD(&b, l);
		m_instructionTime += 4;
	break;

	case 0x46:
		//46		LD B,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecLD(&b, n);
		m_instructionTime += 7;
	break;

	case 0x47:
		//47		LD B,A			4	1	1
		ExecLD(&b, a);
		m_instructionTime += 4;
	break;

	case 0x48:
		//48		LD C,B			4	1	1
		ExecLD(&c, b);
		m_instructionTime += 4;
	break;

	case 0x49:
		//49		LD C,C			4	1	1
		ExecLD(&c, c);
		m_instructionTime += 4;
	break;

	case 0x4a:
		//4A		LD C,D			4	1	1
		ExecLD(&c, d);
		m_instructionTime += 4;
	break;

	case 0x4b:
		//4B		LD C,E			4	1	1
		ExecLD(&c, e);
		m_instructionTime += 4;
	break;

	case 0x4c:
		//4C		LD C,H			4	1	1
		ExecLD(&c, h);
		m_instructionTime += 4;
	break;

	case 0x4d:
		//4D		LD C,L			4	1	1
		ExecLD(&c, l);
		m_instructionTime += 4;
	break;

	case 0x4e:
		//4E		LD C,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecLD(&c, n);
		m_instructionTime += 7;
	break;

	case 0x4f:
		//4F		LD C,A			4	1	1
		ExecLD(&c, a);
		m_instructionTime += 4;
	break;


	// 5x


	case 0x50:
		//50		LD D,B			4	1	1
		ExecLD(&d, b);
		m_instructionTime += 4;
	break;

	case 0x51:
		//51		LD D,C			4	1	1
		ExecLD(&d, c);
		m_instructionTime += 4;
	break;

	case 0x52:
		//52		LD D,D			4	1	1
		ExecLD(&d, d);
		m_instructionTime += 4;
	break;

	case 0x53:
		//53		LD D,E			4	1	1
		ExecLD(&d, e);
		m_instructionTime += 4;
	break;

	case 0x54:
		//54		LD D,H			4	1	1
		ExecLD(&d, h);
		m_instructionTime += 4;
	break;

	case 0x55:
		//55		LD D,L			4	1	1
		ExecLD(&d, l);
		m_instructionTime += 4;
	break;

	case 0x56:
		//56		LD D,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecLD(&d, n);
		m_instructionTime += 7;
	break;

	case 0x57:
		//57		LD D,A			4	1	1
		ExecLD(&d, a);
		m_instructionTime += 4;
	break;

	case 0x58:
		//58		LD E,B			4	1	1
		ExecLD(&e, b);
		m_instructionTime += 4;
	break;

	case 0x59:
		//59		LD E,C			4	1	1
		ExecLD(&e, c);
		m_instructionTime += 4;
	break;

	case 0x5a:
		//5A		LD E,D			4	1	1
		ExecLD(&e, d);
		m_instructionTime += 4;
	break;

	case 0x5b:
		//5B		LD E,E			4	1	1
		ExecLD(&e, e);
		m_instructionTime += 4;
	break;

	case 0x5c:
		//5C		LD E,H			4	1	1
		ExecLD(&e, h);
		m_instructionTime += 4;
	break;

	case 0x5d:
		//5D		LD E,L			4	1	1
		ExecLD(&e, l);
		m_instructionTime += 4;
	break;

	case 0x5e:
		//5E		LD E,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecLD(&e, n);
		m_instructionTime += 7;
	break;

	case 0x5f:
		//5F		LD E,A			4	1	1
		ExecLD(&e, a);
		m_instructionTime += 4;
	break;


	// 6x


	case 0x60:
		//60		LD H,B			4	1	1
		ExecLD(&h, b);
		m_instructionTime += 4;
	break;

	case 0x61:
		//61		LD H,C			4	1	1
		ExecLD(&h, c);
		m_instructionTime += 4;
	break;

	case 0x62:
		//62		LD H,D			4	1	1
		ExecLD(&h, d);
		m_instructionTime += 4;
	break;

	case 0x63:
		//63		LD H,E			4	1	1
		ExecLD(&h, e);
		m_instructionTime += 4;
	break;

	case 0x64:
		//64		LD H,H			4	1	1
		ExecLD(&h, h);
		m_instructionTime += 4;
	break;

	case 0x65:
		//65		LD H,L			4	1	1
		ExecLD(&h, l);
		m_instructionTime += 4;
	break;

	case 0x66:
		//66		LD H,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecLD(&h, n);
		m_instructionTime += 7;
	break;

	case 0x67:
		//67		LD H,A			4	1	1
		ExecLD(&h, a);
		m_instructionTime += 4;
	break;

	case 0x68:
		//68		LD L,B			4	1	1
		ExecLD(&l, b);
		m_instructionTime += 4;
	break;

	case 0x69:
		//69		LD L,C			4	1	1
		ExecLD(&l, c);
		m_instructionTime += 4;
	break;

	case 0x6a:
		//6A		LD L,D			4	1	1
		ExecLD(&l, d);
		m_instructionTime += 4;
	break;

	case 0x6b:
		//6B		LD L,E			4	1	1
		ExecLD(&l, e);
		m_instructionTime += 4;
	break;

	case 0x6c:
		//6C		LD L,H			4	1	1
		ExecLD(&l, h);
		m_instructionTime += 4;
	break;

	case 0x6d:
		//6D		LD L,L			4	1	1
		ExecLD(&l, l);
		m_instructionTime += 4;
	break;

	case 0x6e:
		//6E		LD L,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecLD(&l, n);
		m_instructionTime += 7;
	break;

	case 0x6f:
		//6F		LD L,A			4	1	1
		ExecLD(&l, a);
		m_instructionTime += 4;
	break;


	// 7x


	case 0x70:
		//70		LD (HL),B		7	2	1
		ExecLD(&n, b);
		m_memory->Write8(hl, n);
		m_instructionTime += 7;
	break;

	case 0x71:
		//71		LD (HL),C		7	2	1
		ExecLD(&n, c);
		m_memory->Write8(hl, n);
		m_instructionTime += 7;
	break;

	case 0x72:
		//72		LD (HL),D		7	2	1
		ExecLD(&n, d);
		m_memory->Write8(hl, n);
		m_instructionTime += 7;
	break;

	case 0x73:
		//73		LD (HL),E		7	2	1
		ExecLD(&n, e);
		m_memory->Write8(hl, n);
		m_instructionTime += 7;
	break;

	case 0x74:
		//74		LD (HL),H		7	2	1
		ExecLD(&n, h);
		m_memory->Write8(hl, n);
		m_instructionTime += 7;
	break;

	case 0x75:
		//75		LD (HL),L		7	2	1
		ExecLD(&n, l);
		m_memory->Write8(hl, n);
		m_instructionTime += 7;
	break;

	case 0x76:
		//76		HALT			4	1	1	(repeated till next int)
		m_halted = true;
		m_instructionTime += 4;
	break;

	case 0x77:
		//77		LD (HL),A		7	2	1
		ExecLD(&n, a);
		m_memory->Write8(hl, n);
		m_instructionTime += 7;
	break;

	case 0x78:
		//78		LD A,B			4	1	1
		ExecLD(&a, b);
		m_instructionTime += 4;
	break;

	case 0x79:
		//79		LD A,C			4	1	1
		ExecLD(&a, c);
		m_instructionTime += 4;
	break;

	case 0x7a:
		//7A		LD A,D			4	1	1
		ExecLD(&a, d);
		m_instructionTime += 4;
	break;

	case 0x7b:
		//7B		LD A,E			4	1	1
		ExecLD(&a, e);
		m_instructionTime += 4;
	break;

	case 0x7c:
		//7C		LD A,H			4	1	1
		ExecLD(&a, h);
		m_instructionTime += 4;
	break;

	case 0x7d:
		//7D		LD A,L			4	1	1
		ExecLD(&a, l);
		m_instructionTime += 4;
	break;

	case 0x7e:
		//7E		LD A,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecLD(&a, n);
		m_instructionTime += 7;
	break;

	case 0x7f:
		//7F		LD A,A			4	1	1
		ExecLD(&a, a);
		m_instructionTime += 4;
	break;


	// 8x


	case 0x80:
		//80		ADD A,B			4	1	1
		ExecADD(&a, b);
		m_instructionTime += 4;
	break;

	case 0x81:
		//81		ADD A,C			4	1	1
		ExecADD(&a, c);
		m_instructionTime += 4;
	break;

	case 0x82:
		//82		ADD A,D			4	1	1
		ExecADD(&a, d);
		m_instructionTime += 4;
	break;

	case 0x83:
		//83		ADD A,E			4	1	1
		ExecADD(&a, e);
		m_instructionTime += 4;
	break;

	case 0x84:
		//84		ADD A,H			4	1	1
		ExecADD(&a, h);
		m_instructionTime += 4;
	break;

	case 0x85:
		//85		ADD A,L			4	1	1
		ExecADD(&a, l);
		m_instructionTime += 4;
	break;

	case 0x86:
		//86		ADD A,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecADD(&a, n);
		m_instructionTime += 7;
	break;

	case 0x87:
		//87		ADD A,A			4	1	1
		ExecADD(&a, a);
		m_instructionTime += 4;
	break;

	case 0x88:
		//88		ADC A,B			4	1	1
		ExecADC(&a, b);
		m_instructionTime += 4;
	break;

	case 0x89:
		//89		ADC A,C			4	1	1
		ExecADC(&a, c);
		m_instructionTime += 4;
	break;

	case 0x8a:
		//8A		ADC A,D			4	1	1
		ExecADC(&a, d);
		m_instructionTime += 4;
	break;

	case 0x8b:
		//8B		ADC A,E			4	1	1
		ExecADC(&a, e);
		m_instructionTime += 4;
	break;

	case 0x8c:
		//8C		ADC A,H			4	1	1
		ExecADC(&a, h);
		m_instructionTime += 4;
	break;

	case 0x8d:
		//8D		ADC A,L			4	1	1
		ExecADC(&a, l);
		m_instructionTime += 4;
	break;

	case 0x8e:
		//8E		ADC A,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecADC(&a, n);
		m_instructionTime += 7;
	break;

	case 0x8f:
		//8F		ADC A,A			4	1	1
		ExecADC(&a, a);
		m_instructionTime += 4;
	break;


	// 9x


	case 0x90:
		//90		SUB B			4	1	1
		ExecSUB(b);
		m_instructionTime += 4;
	break;

	case 0x91:
		//91		SUB C			4	1	1
		ExecSUB(c);
		m_instructionTime += 4;
	break;

	case 0x92:
		//92		SUB D			4	1	1
		ExecSUB(d);
		m_instructionTime += 4;
	break;

	case 0x93:
		//93		SUB E			4	1	1
		ExecSUB(e);
		m_instructionTime += 4;
	break;

	case 0x94:
		//94		SUB H			4	1	1
		ExecSUB(h);
		m_instructionTime += 4;
	break;

	case 0x95:
		//95		SUB L			4	1	1
		ExecSUB(l);
		m_instructionTime += 4;
	break;

	case 0x96:
		//96		SUB (HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecSUB(n);
		m_instructionTime += 7;
	break;

	case 0x97:
		//97		SUB A			4	1	1
		ExecSUB(a);
		m_instructionTime += 4;
	break;

	case 0x98:
		//98		SBC A,B			4	1	1
		ExecSBC(&a, b);
		m_instructionTime += 4;
	break;

	case 0x99:
		//99		SBC A,C			4	1	1
		ExecSBC(&a, c);
		m_instructionTime += 4;
	break;

	case 0x9a:
		//9A		SBC A,D			4	1	1
		ExecSBC(&a, d);
		m_instructionTime += 4;
	break;

	case 0x9b:
		//9B		SBC A,E			4	1	1
		ExecSBC(&a, e);
		m_instructionTime += 4;
	break;

	case 0x9c:
		//9C		SBC A,H			4	1	1
		ExecSBC(&a, h);
		m_instructionTime += 4;
	break;

	case 0x9d:
		//9D		SBC A,L			4	1	1
		ExecSBC(&a, l);
		m_instructionTime += 4;
	break;

	case 0x9e:
		//9E		SBC A,(HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecSBC(&a, n);
		m_instructionTime += 7;
	break;

	case 0x9f:
		//9F		SBC A,A			4	1	1
		ExecSBC(&a, a);
		m_instructionTime += 4;
	break;


	// ax


	case 0xa0:
		//A0		AND B			4	1	1
		ExecAND(b);
		m_instructionTime += 4;
	break;

	case 0xa1:
		//A1		AND C			4	1	1
		ExecAND(c);
		m_instructionTime += 4;
	break;

	case 0xa2:
		//A2		AND D			4	1	1
		ExecAND(d);
		m_instructionTime += 4;
	break;

	case 0xa3:
		//A3		AND E			4	1	1
		ExecAND(e);
		m_instructionTime += 4;
	break;

	case 0xa4:
		//A4		AND H			4	1	1
		ExecAND(h);
		m_instructionTime += 4;
	break;

	case 0xa5:
		//A5		AND L			4	1	1
		ExecAND(l);
		m_instructionTime += 4;
	break;

	case 0xa6:
		//A6		AND (HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecAND(n);
		m_instructionTime += 7;
	break;

	case 0xa7:
		//A7		AND A			4	1	1
		ExecAND(a);
		m_instructionTime += 4;
	break;

	case 0xa8:
		//A8		XOR B			4	1	1
		ExecXOR(b);
		m_instructionTime += 4;
	break;

	case 0xa9:
		//A9		XOR C			4	1	1
		ExecXOR(c);
		m_instructionTime += 4;
	break;

	case 0xaa:
		//AA		XOR D			4	1	1
		ExecXOR(d);
		m_instructionTime += 4;
	break;

	case 0xab:
		//AB		XOR E			4	1	1
		ExecXOR(e);
		m_instructionTime += 4;
	break;

	case 0xac:
		//AC		XOR H			4	1	1
		ExecXOR(h);
		m_instructionTime += 4;
	break;

	case 0xad:
		//AD		XOR L			4	1	1
		ExecXOR(l);
		m_instructionTime += 4;
	break;

	case 0xae:
		//AE		XOR (HL)		7	2	1
		n = m_memory->Read8(hl);
		ExecXOR(n);
		m_instructionTime += 7;
	break;

	case 0xaf:
		//AF		XOR A			4	1	1
		ExecAND(b);
		m_instructionTime += 4;
	break;


	// bx


	case 0xb0:
		//B0		OR B			4	1	1
		ExecOR(b);
		m_instructionTime += 4;
	break;

	case 0xb1:
		//B1		OR C			4	1	1
		ExecOR(c);
		m_instructionTime += 4;
	break;

	case 0xb2:
		//B2		OR D			4	1	1
		ExecOR(d);
		m_instructionTime += 4;
	break;

	case 0xb3:
		//B3		OR E			4	1	1
		ExecOR(e);
		m_instructionTime += 4;
	break;

	case 0xb4:
		//B4		OR H			4	1	1
		ExecOR(h);
		m_instructionTime += 4;
	break;

	case 0xb5:
		//B5		OR L			4	1	1
		ExecOR(l);
		m_instructionTime += 4;
	break;

	case 0xb6:
		//B6		OR (HL)			7	2	1
		n = m_memory->Read8(hl);
		ExecOR(n);
		m_instructionTime += 7;
	break;

	case 0xb7:
		//B7		OR A			4	1	1
		ExecOR(a);
		m_instructionTime += 4;
	break;

	case 0xb8:
		//B8		CP B			4	1	1
		ExecCP(b);
		m_instructionTime += 4;
	break;

	case 0xb9:
		//B9		CP C			4	1	1
		ExecCP(c);
		m_instructionTime += 4;
	break;

	case 0xba:
		//BA		CP D			4	1	1
		ExecCP(d);
		m_instructionTime += 4;
	break;

	case 0xbb:
		//BB		CP E			4	1	1
		ExecCP(e);
		m_instructionTime += 4;
	break;

	case 0xbc:
		//BC		CP H			4	1	1
		ExecCP(h);
		m_instructionTime += 4;
	break;

	case 0xbd:
		//BD		CP L			4	1	1
		ExecCP(l);
		m_instructionTime += 4;
	break;

	case 0xbe:
		//BE		CP (HL)			7	2	1
		n = m_memory->Read8(hl);
		ExecCP(n);
		m_instructionTime += 7;
	break;

	case 0xbf:
		//BF		CP A			4	1	1
		ExecCP(a);
		m_instructionTime += 4;
	break;


	// cx


	case 0xc0:
		//C0		RET NZ			11/5	3/1	1/1	(met/not met)
		if(TST_Z)
		{
			m_instructionTime += 5;
		}
		else
		{
			ExecRET();
			m_instructionTime += 11;
		}
	break;

	case 0xc1:
		//C1		POP BC			10	3	1
		ExecPOP(&bc);
		m_instructionTime += 10;
	break;

	case 0xc2:
		//C2 n n		JP NZ,(nn)		10	3	1	(met or not)
		nn = ReadNext16();
		if(TST_Z)
		{
			m_instructionTime += 10;
		}
		else
		{
			ExecJP(nn);
			m_instructionTime += 10;
		}
	break;

	case 0xc3:
		//C3 n n		JP (nn)			10	3	1
		nn = ReadNext16();
		ExecJP(nn);
		m_instructionTime += 10;
	break;

	case 0xc4:
		//C4 n n		CALL NZ,(nn)		17/10	5/3	1/1	(met/not met)
		nn = ReadNext16();
		if(TST_Z)
		{
			m_instructionTime += 10;
		}
		else
		{
			ExecCALL(nn);
			m_instructionTime += 17;
		}
	break;

	case 0xc5:
		//C5		PUSH BC			11	3	1
		ExecPUSH(&bc);
		m_instructionTime += 11;
	break;

	case 0xc6:
		//C6 n		ADD A,n			7	2	1
		n = ReadNext8();
		ExecADD(&a, n);
		m_instructionTime += 7;
	break;

	case 0xc7:
		//C7		RST 0H			11	3	1
		ExecRST((u16)0x00);
		m_instructionTime += 11;
	break;

	case 0xc8:
		//C8		RET Z			11/5	3/1	1/1	(met/not met)
		if(TST_Z)
		{
			ExecRET();
			m_instructionTime += 11;
		}
		else
		{
			m_instructionTime += 5;
		}
	break;

	case 0xc9:
		//C9		RET			10	3	1
		ExecRET();
		m_instructionTime += 10;
	break;

	case 0xca:
		//CA n n		JP Z,(nn)		10	3	1	(always same)
		nn = ReadNext16();
		tt = m_memory->Read16(nn);
		if(TST_Z)
		{
			ExecJP(tt);
			m_instructionTime += 10;
		}
		else
		{
			m_instructionTime += 10;
		}
	break;

	case 0xcb:
		//Handled by ExecCB.  We should never get here.
	break;

	case 0xcc:
		//CC n n		CALL Z,(nn)		17/10	5/3	1/1	(met/not met)
		nn = ReadNext16();
		tt = m_memory->Read16(nn);
		if(TST_Z)
		{
			ExecCALL(tt);
			m_instructionTime += 17;
		}
		else
		{
			m_instructionTime += 10;
		}
	break;

	case 0xcd:
		//CD n n		CALL (nn)		17	5	1
		nn = ReadNext16();
		tt = m_memory->Read16(nn);
		ExecCALL(tt);
		m_instructionTime += 17;
	break;

	case 0xce:
		//CE n		ADC A,n			7	2	1
		n = ReadNext8();
		ExecADC(&a, n);
		m_instructionTime += 7;
	break;

	case 0xcf:
		//CF		RST 8H			11	3	1
		ExecRST((u16)0x08);
		m_instructionTime += 11;
	break;


	// dx


	case 0xd0:
		//D0		RET NC			5	1	1
		if(TST_C)
		{
			m_instructionTime += 5;
		}
		else
		{
			ExecRET();
			m_instructionTime += 11;
		}
	break;

	case 0xd1:
		//D1		POP DE			10	3	1
		ExecPOP(&de);
		m_instructionTime += 10;
	break;

	case 0xd2:
		//D2 n n		JP NC,(nn)		10	3	1	(met or not)
		nn = ReadNext16();
		if(TST_C)
		{
			m_instructionTime += 10;
		}
		else
		{
			ExecJP(nn);
			m_instructionTime += 10;
		}
	break;

	case 0xd3:
		//D3      OUT  (n),A      -
		//?? undocumented
	break;

	case 0xd4:
		//D4 n n		CALL NC,(nn)		17/10	5/3	1/1	(met/not met)
		nn = ReadNext16();
		tt = m_memory->Read16(nn);
		if(TST_C)
		{
			m_instructionTime += 10;
		}
		else
		{
			ExecCALL(tt);
			m_instructionTime += 17;
		}
	break;

	case 0xd5:
		//D5		PUSH DE			11	3	1
		ExecPUSH(&de);
		m_instructionTime += 11;
	break;

	case 0xd6:
		//D6 n		SUB n			7	2	1
		n = ReadNext8();
		ExecSUB(n);
		m_instructionTime += 7;
	break;

	case 0xd7:
		//D7		RST 10H			11	3	1
		ExecRST((u16)0x10);
		m_instructionTime += 11;
	break;

	case 0xd8:
		//D8		RET C			5	1	1
		if(TST_C)
		{
			ExecRET();
			m_instructionTime += 11;
		}
		else
		{
			m_instructionTime += 5;
		}
	break;

	case 0xd9:
		//D9      EXX             RETI
		ExecRET();
		m_interruptsEnabled = true;
		m_instructionTime += 10;	//?? guessing from RET.
	break;

	case 0xda:
		//DA n n		JP C,(nn)		10	3	1	(met or not)
		nn = ReadNext16();
		tt = m_memory->Read16(nn);
		if(TST_C)
		{
			ExecJP(tt);
			m_instructionTime += 10;
		}
		else
		{
			m_instructionTime += 10;
		}
	break;

	case 0xdb:
		//DB      IN   A,(n)      -
		//?? undocumented
	break;

	case 0xdc:
		//DC n n		CALL C,(nn)		17/10	5/3	1
		nn = ReadNext16();
		tt = m_memory->Read16(nn);
		if(TST_C)
		{
			ExecCALL(tt);
			m_instructionTime += 17;
		}
		else
		{
			m_instructionTime += 10;
		}
	break;

	case 0xdd:
		//DD      <IX>            -
		//?? undocumented
	break;

	case 0xde:
		//DE n		SBC A,n
		n = ReadNext8();
		ExecSBC(&a, n);
		m_instructionTime += 7;
	break;

	case 0xdf:
		//DF		RST 18H
		ExecRST((u16)0x18);
		m_instructionTime += 11;
	break;


	// ex


	case 0xe0:
		//E0      RET  PO         LD   (FF00+n),A
		n = ReadNext8();
		address = 0xff00 + n;
		ExecLD(&t, a);
		m_memory->Write8(address, t);
	break;

	case 0xe1:
		//E1		POP HL
		ExecPOP(&hl);
		m_instructionTime += 10;
	break;

	case 0xe2:
		//E2      JP   PO,nn      LD   (FF00+C),A
		address = 0xff00 + c;
		ExecLD(&t, a);
		m_memory->Write8(address, t);
	break;

	case 0xe3:
		//E3      EX   (SP),HL    -
		//?? undocumented
	break;

	case 0xe4:
		//E4      CALL P0,nn      -
		//?? undocumented
	break;

	case 0xe5:
		//E5		PUSH HL
		ExecPUSH(&hl);
		m_instructionTime += 11;
	break;

	case 0xe6:
		//E6 n		AND n
		n = ReadNext8();
		ExecAND(n);
		m_instructionTime += 7;
	break;

	case 0xe7:
		//E7		RST 20H
		ExecRST((u16)0x20);
	break;

	case 0xe8:
	{
		//E8      RET  PE         ADD  SP,dd
		//?? No idea if this method will handle flags correctly
		s8 sn = (s8)ReadNext8();
		s16 operand = sn;
		ExecADD(&sp, (u16)operand);
		m_instructionTime += 14;	//?? guessing based on ADD HL, ss (11) + 3 for memory read
	}
	break;

	case 0xe9:
		//E9		JP (HL)
		ExecJP(hl);
		m_instructionTime += 4;
	break;

	case 0xea:
		//EA      JP   PE,nn      LD   (nn),A
		nn = ReadNext16();
		ExecLD(&t, a);
		m_memory->Write8(nn, t);
		m_instructionTime += 13;
	break;

	case 0xeb:
		//EB      EX   DE,HL      -
		//?? undocumented
	break;

	case 0xec:
		//EC      CALL PE,nn      -
		//?? undocumented
	break;

	case 0xed:
		//ED      <pref>          -
		//?? undocumented
	break;

	case 0xee:
		//EE n		XOR n
		n = ReadNext8();
		ExecXOR(n);
		m_instructionTime += 7;
	break;

	case 0xef:
		//EF		RST 28H
		ExecRST((u16)0x28);
	break;


	// fx


	case 0xf0:
		//F0      RET  P          LD   A,(FF00+n)
		n = ReadNext8();
		address = 0xff00 + n;
		t = m_memory->Read8(address);
		ExecLD(&a, t);
	break;

	case 0xf1:
		//F1		POP AF
		ExecPOP(&af);
		m_instructionTime += 10;
	break;

	case 0xf2:
		//F2      JP   P,nn       LD   A,(FF00+C)
		address = 0xff00 + c;
		t = m_memory->Read8(address);
		ExecLD(&a, t);
	break;

	case 0xf3:
		//F3		DI
		m_interruptsEnabled = false;
		m_instructionTime += 4;
	break;

	case 0xf4:
		//F4      CALL P,nn       -
		//?? undocumented
	break;

	case 0xf5:
		//F5		PUSH AF
		ExecPUSH(&af);
		m_instructionTime += 11;
	break;

	case 0xf6:
		//F6 n		OR n
		n = ReadNext8();
		ExecOR(n);
		m_instructionTime += 7;
	break;

	case 0xf7:
		//F7		RST 30H
		ExecRST((u16)0x30);
	break;

	case 0xf8:
	{
		//F8      RET  M          LD   HL,SP+dd
		s8 offset = (s8)ReadNext8();
		address = sp + offset;
		ExecLD(&hl, address);
		m_instructionTime += 13;	//?? wild guess
	}
	break;

	case 0xf9:
		//F9		LD SP,HL
		ExecLD(&sp, hl);
		m_instructionTime += 6;
	break;

	case 0xfa:
		//FA      JP   M,nn       LD   A,(nn)
		nn = ReadNext16();
		t = m_memory->Read8(nn);
		ExecLD(&a, t);
		m_instructionTime += 13;
	break;

	case 0xfb:
		//FB		EI
		ExecEI();
		m_instructionTime += 4;
	break;

	case 0xfc:
		//FC      CALL M,nn       -
		//?? undocumented
	break;

	case 0xfd:
		//FD      <IY>            -
		//?? undocumented
	break;

	case 0xfe:
		//FE n		CP n
		n = ReadNext8();
		ExecCP(n);
		m_instructionTime += 7;
	break;

	case 0xff:
		//FF		RST 38H
		ExecRST((u16)0x38);
		m_instructionTime += 11;
	break;



	default:
	break;
	}


	return m_instructionTime;
}

int CPU::ExecuteCB()
{
	u8 n,t;
	u16 nn,tt;
	u16 address;

	u8 opcode = m_memory->Read8(pc++);

	switch(opcode)
	{
	case 0x00:
		//CB00		RLC B			8	2	2
		ExecRLC(&b);
		m_instructionTime += 8;
	break;

	case 0x01:
		//CB01		RLC C			8	2	2
		ExecRLC(&c);
		m_instructionTime += 8;
	break;

	case 0x02:
		//CB02		RLC D			8	2	2
		ExecRLC(&d);
		m_instructionTime += 8;
	break;

	case 0x03:
		//CB03		RLC E			8	2	2
		ExecRLC(&e);
		m_instructionTime += 8;
	break;

	case 0x04:
		//CB04		RLC H			8	2	2
		ExecRLC(&h);
		m_instructionTime += 8;
	break;

	case 0x05:
		//CB05		RLC L			8	2	2
		ExecRLC(&l);
	break;

	case 0x06:
		//CB06		RLC (HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRLC(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x07:
		//CB07		RLC A			8	2	2
		ExecRLC(&a);
		m_instructionTime += 8;
	break;

	case 0x08:
		//CB08		RRC B			8	2	2
		ExecRRC(&b);
		m_instructionTime += 8;
	break;

	case 0x09:
		//CB09		RRC C			8	2	2
		ExecRRC(&c);
		m_instructionTime += 8;
	break;

	case 0x0A:
		//CB0A		RRC D			8	2	2
		ExecRRC(&d);
		m_instructionTime += 8;
	break;

	case 0x0B:
		//CB0B		RRC E			8	2	2
		ExecRRC(&e);
		m_instructionTime += 8;
	break;

	case 0x0C:
		//CB0C		RRC H			8	2	2
		ExecRRC(&h);
		m_instructionTime += 8;
	break;

	case 0x0D:
		//CB0D		RRC L			8	2	2
		ExecRRC(&l);
		m_instructionTime += 8;
	break;

	case 0x0E:
		//CB0E		RRC (HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRRC(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x0F:
		//CB0F		RRC A			8	2	2
		ExecRRC(&a);
		m_instructionTime += 8;
	break;

	case 0x10:
		//CB10		RL B			8	2	2
		ExecRL(&b);
		m_instructionTime += 8;
	break;

	case 0x11:
		//CB11		RL C			8	2	2
		ExecRL(&c);
		m_instructionTime += 8;
	break;

	case 0x12:
		//CB12		RL D			8	2	2
		ExecRL(&d);
		m_instructionTime += 8;
	break;

	case 0x13:
		//CB13		RL E			8	2	2
		ExecRL(&e);
		m_instructionTime += 8;
	break;

	case 0x14:
		//CB14		RL H			8	2	2
		ExecRL(&h);
		m_instructionTime += 8;
	break;

	case 0x15:
		//CB15		RL L			8	2	2
		ExecRL(&l);
		m_instructionTime += 8;
	break;

	case 0x16:
		//CB16		RL (HL)			15	4	2
		n = m_memory->Read8(hl);
		ExecRL(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x17:
		//CB17		RL A			8	2	2
		ExecRL(&a);
		m_instructionTime += 8;
	break;

	case 0x18:
		//CB18		RR B			8	2	2
		ExecRR(&b);
		m_instructionTime += 8;
	break;

	case 0x19:
		//CB19		RR C			8	2	2
		ExecRR(&c);
		m_instructionTime += 8;
	break;

	case 0x1A:
		//CB1A		RR D			8	2	2
		ExecRR(&d);
		m_instructionTime += 8;
	break;

	case 0x1B:
		//CB1B		RR E			8	2	2
		ExecRR(&e);
		m_instructionTime += 8;
	break;

	case 0x1C:
		//CB1C		RR H			8	2	2
		ExecRR(&h);
		m_instructionTime += 8;
	break;

	case 0x1D:
		//CB1D		RR L			8	2	2
		ExecRR(&l);
		m_instructionTime += 8;
	break;

	case 0x1E:
		//CB1E		RR (HL)			15	4	2
		n = m_memory->Read8(hl);
		ExecRR(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x1F:
		//CB1F		RR A			8	2	2
		ExecRR(&a);
		m_instructionTime += 8;
	break;

	case 0x20:
		//CB20		SLA B			8	2	2
		ExecSLA(&b);
		m_instructionTime += 8;
	break;

	case 0x21:
		//CB21		SLA C			8	2	2
		ExecSLA(&c);
		m_instructionTime += 8;
	break;

	case 0x22:
		//CB22		SLA D			8	2	2
		ExecSLA(&d);
		m_instructionTime += 8;
	break;

	case 0x23:
		//CB23		SLA E			8	2	2
		ExecSLA(&e);
		m_instructionTime += 8;
	break;

	case 0x24:
		//CB24		SLA H			8	2	2
		ExecSLA(&h);
		m_instructionTime += 8;
	break;

	case 0x25:
		//CB25		SLA L			8	2	2
		ExecSLA(&l);
		m_instructionTime += 8;
	break;

	case 0x26:
		//CB26		SLA (HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSLA(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x27:
		//CB27		SLA A			8	2	2
		ExecSLA(&a);
		m_instructionTime += 8;
	break;

	case 0x28:
		//CB28		SRA B			8	2	2
		ExecSRA(&b);
		m_instructionTime += 8;
	break;

	case 0x29:
		//CB29		SRA C			8	2	2
		ExecSRA(&c);
		m_instructionTime += 8;
	break;

	case 0x2A:
		//CB2A		SRA D			8	2	2
		ExecSRA(&d);
		m_instructionTime += 8;
	break;

	case 0x2B:
		//CB2B		SRA E			8	2	2
		ExecSRA(&e);
		m_instructionTime += 8;
	break;

	case 0x2C:
		//CB2C		SRA H			8	2	2
		ExecSRA(&h);
		m_instructionTime += 8;
	break;

	case 0x2D:
		//CB2D		SRA L			8	2	2
		ExecSRA(&l);
		m_instructionTime += 8;
	break;

	case 0x2E:
		//CB2E		SRA (HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSRA(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x2F:
		//CB2F		SRA A			8	2	2
		ExecSRA(&a);
		m_instructionTime += 8;
	break;

	case 0x30:
		//CB30		SLL B*		SWAP B			8	2	2
		ExecSWAP(&b);
		m_instructionTime += 8;
	break;

	case 0x31:
		//CB31		SLL C*		SWAP C			8	2	2
		ExecSWAP(&c);
		m_instructionTime += 8;
	break;

	case 0x32:
		//CB32		SLL D*		SWAP D			8	2	2
		ExecSWAP(&d);
		m_instructionTime += 8;
	break;

	case 0x33:
		//CB33		SLL E*		SWAP E			8	2	2
		ExecSWAP(&e);
		m_instructionTime += 8;
	break;

	case 0x34:
		//CB34		SLL H*		SWAP H			8	2	2
		ExecSWAP(&h);
		m_instructionTime += 8;
	break;

	case 0x35:
		//CB35		SLL L*		SWAP L			8	2	2
		ExecSWAP(&l);
		m_instructionTime += 8;
	break;

	case 0x36:
		//CB36		SLL (HL)*	SWAP (HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSWAP(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x37:
		//CB37		SLL A*		SWAP A			8	2	2
		ExecSWAP(&a);
		m_instructionTime += 8;
	break;

	case 0x38:
		//CB38		SRL B			8	2	2
		ExecSRL(&b);
		m_instructionTime += 8;
	break;

	case 0x39:
		//CB39		SRL C			8	2	2
		ExecSRL(&c);
		m_instructionTime += 8;
	break;

	case 0x3A:
		//CB3A		SRL D			8	2	2
		ExecSRL(&d);
		m_instructionTime += 8;
	break;

	case 0x3B:
		//CB3B		SRL E			8	2	2
		ExecSRL(&e);
		m_instructionTime += 8;
	break;

	case 0x3C:
		//CB3C		SRL H			8	2	2
		ExecSRL(&h);
		m_instructionTime += 8;
	break;

	case 0x3D:
		//CB3D		SRL L			8	2	2
		ExecSRL(&l);
		m_instructionTime += 8;
	break;

	case 0x3E:
		//CB3E		SRL (HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSRL(&n);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x3F:
		//CB3F		SRL A			8	2	2
		ExecSRL(&a);
	break;

	case 0x40:
		//CB40		BIT 0,B			8	2	2
		ExecBIT(b,0);
		m_instructionTime += 8;
	break;

	case 0x41:
		//CB41		BIT 0,C			8	2	2
		ExecBIT(c,0);
		m_instructionTime += 8;
	break;

	case 0x42:
		//CB42		BIT 0,D			8	2	2
		ExecBIT(d,0);
		m_instructionTime += 8;
	break;

	case 0x43:
		//CB43		BIT 0,E			8	2	2
		ExecBIT(e,0);
		m_instructionTime += 8;
	break;

	case 0x44:
		//CB44	 	BIT 0,H			8	2	2
		ExecBIT(h,0);
		m_instructionTime += 8;
	break;

	case 0x45:
		//CB45	 	BIT 0,L			8	2	2
		ExecBIT(l,0);
		m_instructionTime += 8;
	break;

	case 0x46:
		//CB46	 	BIT 0,(HL)		12	3	2
		n = m_memory->Read8(hl);
		ExecBIT(n,0);
		m_instructionTime += 12;
	break;

	case 0x47:
		//CB47	 	BIT 0,A			8	2	2
		ExecBIT(a,0);
		m_instructionTime += 8;
	break;

	case 0x48:
		//CB48		BIT 1,B			8	2	2
		ExecBIT(b,1);
		m_instructionTime += 8;
	break;

	case 0x49:
		//CB49	 	BIT 1,C			8	2	2
		ExecBIT(c,1);
		m_instructionTime += 8;
	break;

	case 0x4A:
		//CB4A	 	BIT 1,D			8	2	2
		ExecBIT(d,1);
		m_instructionTime += 8;
	break;

	case 0x4B:
		//CB4B	 	BIT 1,E			8	2	2
		ExecBIT(e,1);
		m_instructionTime += 8;
	break;

	case 0x4C:
		//CB4C	 	BIT 1,H			8	2	2
		ExecBIT(h,1);
		m_instructionTime += 8;
	break;

	case 0x4D:
		//CB4D	 	BIT 1,L			8	2	2
		ExecBIT(l,1);
		m_instructionTime += 8;
	break;

	case 0x4E:
		//CB4E	 	BIT 1,(HL)		12	3	2
		n = m_memory->Read8(hl);
		ExecBIT(n,1);
		m_instructionTime += 12;
	break;

	case 0x4F:
		//CB4F	 	BIT 1,A			8	2	2
		ExecBIT(a,1);
		m_instructionTime += 8;
	break;

	case 0x50:
		//CB50		BIT 2,B			8	2	2
		ExecBIT(b,2);
		m_instructionTime += 8;
	break;

	case 0x51:
		//CB51	 	BIT 2,C			8	2	2
		ExecBIT(c,2);
		m_instructionTime += 8;
	break;

	case 0x52:
		//CB52	 	BIT 2,D			8	2	2
		ExecBIT(d,2);
		m_instructionTime += 8;
	break;

	case 0x53:
		//CB53	 	BIT 2,E			8	2	2
		ExecBIT(e,2);
		m_instructionTime += 8;
	break;

	case 0x54:
		//CB54	 	BIT 2,H			8	2	2
		ExecBIT(h,2);
		m_instructionTime += 8;
	break;

	case 0x55:
		//CB55	 	BIT 2,L			8	2	2
		ExecBIT(l,2);
		m_instructionTime += 8;
	break;

	case 0x56:
		//CB56	 	BIT 2,(HL)		12	3	2
		n = m_memory->Read8(hl);
		ExecBIT(n,2);
		m_instructionTime += 12;
	break;

	case 0x57:
		//CB57	 	BIT 2,A			8	2	2
		ExecBIT(a,2);
		m_instructionTime += 8;
	break;

	case 0x58:
		//CB58		BIT 3,B			8	2	2
		ExecBIT(b,3);
		m_instructionTime += 8;
	break;

	case 0x59:
		//CB59	 	BIT 3,C			8	2	2
		ExecBIT(c,3);
		m_instructionTime += 8;
	break;

	case 0x5A:
		//CB5A	 	BIT 3,D			8	2	2
		ExecBIT(d,3);
		m_instructionTime += 8;
	break;

	case 0x5B:
		//CB5B	 	BIT 3,E			8	2	2
		ExecBIT(e,3);
		m_instructionTime += 8;
	break;

	case 0x5C:
		//CB5C	 	BIT 3,H			8	2	2
		ExecBIT(h,3);
		m_instructionTime += 8;
	break;

	case 0x5D:
		//CB5D	 	BIT 3,L			8	2	2
		ExecBIT(l,3);
		m_instructionTime += 8;
	break;

	case 0x5E:
		//CB5E	 	BIT 3,(HL)		12	3	2
		n = m_memory->Read8(hl);
		ExecBIT(n,3);
		m_instructionTime += 12;
	break;

	case 0x5F:
		//CB5F	 	BIT 3,A			8	2	2
		ExecBIT(a,3);
		m_instructionTime += 8;
	break;

	case 0x60:
		//CB60		BIT 4,B			8	2	2
		ExecBIT(b,4);
		m_instructionTime += 8;
	break;

	case 0x61:
		//CB61	 	BIT 4,C			8	2	2
		ExecBIT(c,4);
		m_instructionTime += 8;
	break;

	case 0x62:
		//CB62	 	BIT 4,D			8	2	2
		ExecBIT(d,4);
		m_instructionTime += 8;
	break;

	case 0x63:
		//CB63	 	BIT 4,E			8	2	2
		ExecBIT(e,4);
		m_instructionTime += 8;
	break;

	case 0x64:
		//CB64	 	BIT 4,H			8	2	2
		ExecBIT(h,4);
		m_instructionTime += 8;
	break;

	case 0x65:
		//CB65	 	BIT 4,L			8	2	2
		ExecBIT(l,4);
		m_instructionTime += 8;
	break;

	case 0x66:
		//CB66	 	BIT 4,(HL)		12	3	2
		n = m_memory->Read8(hl);
		ExecBIT(n,4);
		m_instructionTime += 12;
	break;

	case 0x67:
		//CB67	 	BIT 4,A			8	2	2
		ExecBIT(a,4);
		m_instructionTime += 8;
	break;

	case 0x68:
		//CB68		BIT 5,B			8	2	2
		ExecBIT(b,5);
		m_instructionTime += 8;
	break;

	case 0x69:
		//CB69	 	BIT 5,C			8	2	2
		ExecBIT(c,5);
		m_instructionTime += 8;
	break;

	case 0x6A:
		//CB6A	 	BIT 5,D			8	2	2
		ExecBIT(d,5);
		m_instructionTime += 8;
	break;

	case 0x6B:
		//CB6B	 	BIT 5,E			8	2	2
		ExecBIT(e,5);
		m_instructionTime += 8;
	break;

	case 0x6C:
		//CB6C	 	BIT 5,H			8	2	2
		ExecBIT(h,5);
		m_instructionTime += 8;
	break;

	case 0x6D:
		//CB6D	 	BIT 5,L			8	2	2
		ExecBIT(l,5);
		m_instructionTime += 8;
	break;

	case 0x6E:
		//CB6E	 	BIT 5,(HL)		12	3	2
		n = m_memory->Read8(hl);
		ExecBIT(n,5);
		m_instructionTime += 12;
	break;

	case 0x6F:
		//CB6F	 	BIT 5,A			8	2	2
		ExecBIT(a,5);
		m_instructionTime += 8;
	break;

	case 0x70:
		//CB70		BIT 6,B			8	2	2
		ExecBIT(b,6);
		m_instructionTime += 8;
	break;

	case 0x71:
		//CB71	 	BIT 6,C			8	2	2
		ExecBIT(c,6);
		m_instructionTime += 8;
	break;

	case 0x72:
		//CB72	 	BIT 6,D			8	2	2
		ExecBIT(d,6);
		m_instructionTime += 8;
	break;

	case 0x73:
		//CB73	 	BIT 6,E			8	2	2
		ExecBIT(e,6);
		m_instructionTime += 8;
	break;

	case 0x74:
		//CB74	 	BIT 6,H			8	2	2
		ExecBIT(h,6);
		m_instructionTime += 8;
	break;

	case 0x75:
		//CB75	 	BIT 6,L			8	2	2
		ExecBIT(l,6);
		m_instructionTime += 8;
	break;

	case 0x76:
		//CB76	 	BIT 6,(HL)		12	3	2
		n = m_memory->Read8(hl);
		ExecBIT(n,6);
		m_instructionTime += 12;
	break;

	case 0x77:
		//CB77	 	BIT 6,A			8	2	2
		ExecBIT(a,6);
		m_instructionTime += 8;
	break;

	case 0x78:
		//CB78		BIT 7,B			8	2	2
		ExecBIT(b,7);
		m_instructionTime += 8;
	break;

	case 0x79:
		//CB79	 	BIT 7,C			8	2	2
		ExecBIT(c,7);
		m_instructionTime += 8;
	break;

	case 0x7A:
		//CB7A	 	BIT 7,D			8	2	2
		ExecBIT(d,7);
		m_instructionTime += 8;
	break;

	case 0x7B:
		//CB7B	 	BIT 7,E			8	2	2
		ExecBIT(e,7);
		m_instructionTime += 8;
	break;

	case 0x7C:
		//CB7C	 	BIT 7,H			8	2	2
		ExecBIT(h,7);
		m_instructionTime += 8;
	break;

	case 0x7D:
		//CB7D	 	BIT 7,L			8	2	2
		ExecBIT(l,7);
		m_instructionTime += 8;
	break;

	case 0x7E:
		//CB7E	 	BIT 7,(HL)		12	3	2
		n = m_memory->Read8(hl);
		ExecBIT(n,7);
		m_instructionTime += 12;
	break;

	case 0x7F:
		//CB7F	 	BIT 7,A			8	2	2
		ExecBIT(a,7);
		m_instructionTime += 8;
	break;

	case 0x80:
		//CB80		RES 0,B			8	2	2
		ExecRES(&b,0);
		m_instructionTime += 8;
	break;

	case 0x81:
		//CB81		RES 0,C			8	2	2
		ExecRES(&c,0);
		m_instructionTime += 8;
	break;

	case 0x82:
		//CB82		RES 0,D			8	2	2
		ExecRES(&d,0);
		m_instructionTime += 8;
	break;

	case 0x83:
		//CB83		RES 0,E			8	2	2
		ExecRES(&e,0);
		m_instructionTime += 8;
	break;

	case 0x84:
		//CB84	 	RES 0,H			8	2	2
		ExecRES(&h,0);
		m_instructionTime += 8;
	break;

	case 0x85:
		//CB85	 	RES 0,L			8	2	2
		ExecRES(&l,0);
		m_instructionTime += 8;
	break;

	case 0x86:
		//CB86	 	RES 0,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRES(&n,0);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x87:
		//CB87	 	RES 0,A			8	2	2
		ExecRES(&a,0);
		m_instructionTime += 8;
	break;

	case 0x88:
		//CB88		RES 1,B			8	2	2
		ExecRES(&b,1);
		m_instructionTime += 8;
	break;

	case 0x89:
		//CB89	 	RES 1,C			8	2	2
		ExecRES(&c,1);
		m_instructionTime += 8;
	break;

	case 0x8A:
		//CB8A	 	RES 1,D			8	2	2
		ExecRES(&d,1);
		m_instructionTime += 8;
	break;

	case 0x8B:
		//CB8B	 	RES 1,E			8	2	2
		ExecRES(&e,1);
		m_instructionTime += 8;
	break;

	case 0x8C:
		//CB8C	 	RES 1,H			8	2	2
		ExecRES(&h,1);
		m_instructionTime += 8;
	break;

	case 0x8D:
		//CB8D	 	RES 1,L			8	2	2
		ExecRES(&l,1);
		m_instructionTime += 8;
	break;

	case 0x8E:
		//CB8E	 	RES 1,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRES(&n,1);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x8F:
		//CB8F	 	RES 1,A			8	2	2
		ExecRES(&a,1);
		m_instructionTime += 8;
	break;

	case 0x90:
		//CB90		RES 2,B			8	2	2
		ExecRES(&b,2);
		m_instructionTime += 8;
	break;

	case 0x91:
		//CB91	 	RES 2,C			8	2	2
		ExecRES(&c,2);
		m_instructionTime += 8;
	break;

	case 0x92:
		//CB92	 	RES 2,D			8	2	2
		ExecRES(&d,2);
		m_instructionTime += 8;
	break;

	case 0x93:
		//CB93	 	RES 2,E			8	2	2
		ExecRES(&e,2);
		m_instructionTime += 8;
	break;

	case 0x94:
		//CB94	 	RES 2,H			8	2	2
		ExecRES(&h,2);
		m_instructionTime += 8;
	break;

	case 0x95:
		//CB95	 	RES 2,L			8	2	2
		ExecRES(&l,2);
		m_instructionTime += 8;
	break;

	case 0x96:
		//CB96	 	RES 2,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRES(&n,2);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x97:
		//CB97	 	RES 2,A			8	2	2
		ExecRES(&a,2);
		m_instructionTime += 8;
	break;

	case 0x98:
		//CB98		RES 3,B			8	2	2
		ExecRES(&b,3);
		m_instructionTime += 8;
	break;

	case 0x99:
		//CB99	 	RES 3,C			8	2	2
		ExecRES(&c,3);
		m_instructionTime += 8;
	break;

	case 0x9A:
		//CB9A	 	RES 3,D			8	2	2
		ExecRES(&d,3);
		m_instructionTime += 8;
	break;

	case 0x9B:
		//CB9B	 	RES 3,E			8	2	2
		ExecRES(&e,3);
		m_instructionTime += 8;
	break;

	case 0x9C:
		//CB9C	 	RES 3,H			8	2	2
		ExecRES(&h,3);
		m_instructionTime += 8;
	break;

	case 0x9D:
		//CB9D	 	RES 3,L			8	2	2
		ExecRES(&l,3);
		m_instructionTime += 8;
	break;

	case 0x9E:
		//CB9E	 	RES 3,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRES(&n,3);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0x9F:
		//CB9F	 	RES 3,A			8	2	2
		ExecRES(&a,3);
		m_instructionTime += 8;
	break;

	case 0xA0:
		//CBA0		RES 4,B			8	2	2
		ExecRES(&b,4);
		m_instructionTime += 8;
	break;

	case 0xA1:
		//CBA1	 	RES 4,C			8	2	2
		ExecRES(&c,4);
		m_instructionTime += 8;
	break;

	case 0xA2:
		//CBA2	 	RES 4,D			8	2	2
		ExecRES(&d,4);
		m_instructionTime += 8;
	break;

	case 0xA3:
		//CBA3	 	RES 4,E			8	2	2
		ExecRES(&e,4);
		m_instructionTime += 8;
	break;

	case 0xA4:
		//CBA4	 	RES 4,H			8	2	2
		ExecRES(&h,4);
		m_instructionTime += 8;
	break;

	case 0xA5:
		//CBA5	 	RES 4,L			8	2	2
		ExecRES(&l,4);
		m_instructionTime += 8;
	break;

	case 0xA6:
		//CBA6	 	RES 4,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRES(&n,4);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xA7:
		//CBA7	 	RES 4,A			8	2	2
		ExecRES(&a,4);
		m_instructionTime += 8;
	break;

	case 0xA8:
		//CBA8		RES 5,B			8	2	2
		ExecRES(&b,5);
		m_instructionTime += 8;
	break;

	case 0xA9:
		//CBA9	 	RES 5,C			8	2	2
		ExecRES(&c,5);
		m_instructionTime += 8;
	break;

	case 0xAA:
		//CBAA	 	RES 5,D			8	2	2
		ExecRES(&d,5);
		m_instructionTime += 8;
	break;

	case 0xAB:
		//CBAB	 	RES 5,E			8	2	2
		ExecRES(&e,5);
		m_instructionTime += 8;
	break;

	case 0xAC:
		//CBAC	 	RES 5,H			8	2	2
		ExecRES(&h,5);
		m_instructionTime += 8;
	break;

	case 0xAD:
		//CBAD	 	RES 5,L			8	2	2
		ExecRES(&l,5);
		m_instructionTime += 8;
	break;

	case 0xAE:
		//CBAE	 	RES 5,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRES(&n,5);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xAF:
		//CBAF	 	RES 5,A			8	2	2
		ExecRES(&a,5);
		m_instructionTime += 8;
	break;

	case 0xB0:
		//CBB0		RES 6,B			8	2	2
		ExecRES(&b,6);
		m_instructionTime += 8;
	break;

	case 0xB1:
		//CBB1	 	RES 6,C			8	2	2
		ExecRES(&c,6);
		m_instructionTime += 8;
	break;

	case 0xB2:
		//CBB2	 	RES 6,D			8	2	2
		ExecRES(&d,6);
		m_instructionTime += 8;
	break;

	case 0xB3:
		//CBB3	 	RES 6,E			8	2	2
		ExecRES(&e,6);
		m_instructionTime += 8;
	break;

	case 0xB4:
		//CBB4	 	RES 6,H			8	2	2
		ExecRES(&h,6);
		m_instructionTime += 8;
	break;

	case 0xB5:
		//CBB5	 	RES 6,L			8	2	2
		ExecRES(&l,6);
		m_instructionTime += 8;
	break;

	case 0xB6:
		//CBB6	 	RES 6,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRES(&n,6);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xB7:
		//CBB7	 	RES 6,A			8	2	2
		ExecRES(&a,6);
		m_instructionTime += 8;
	break;

	case 0xB8:
		//CBB8		RES 7,B			8	2	2
		ExecRES(&b,7);
		m_instructionTime += 8;
	break;

	case 0xB9:
		//CBB9	 	RES 7,C			8	2	2
		ExecRES(&c,7);
		m_instructionTime += 8;
	break;

	case 0xBA:
		//CBBA	 	RES 7,D			8	2	2
		ExecRES(&d,7);
		m_instructionTime += 8;
	break;

	case 0xBB:
		//CBBB	 	RES 7,E			8	2	2
		ExecRES(&e,7);
		m_instructionTime += 8;
	break;

	case 0xBC:
		//CBBC	 	RES 7,H			8	2	2
		ExecRES(&h,7);
		m_instructionTime += 8;
	break;

	case 0xBD:
		//CBBD	 	RES 7,L			8	2	2
		ExecRES(&l,7);
		m_instructionTime += 8;
	break;

	case 0xBE:
		//CBBE	 	RES 7,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecRES(&n,7);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xBF:
		//CBBF	 	RES 7,A			8	2	2
		ExecRES(&a,7);
		m_instructionTime += 8;
	break;

	case 0xC0:
		//CBC0		SET 0,B			8	2	2
		ExecSET(&b,0);
		m_instructionTime += 8;
	break;

	case 0xC1:
		//CBC1		SET 0,C			8	2	2
		ExecSET(&c,0);
		m_instructionTime += 8;
	break;

	case 0xC2:
		//CBC2		SET 0,D			8	2	2
		ExecSET(&d,0);
		m_instructionTime += 8;
	break;

	case 0xC3:
		//CBC3		SET 0,E			8	2	2
		ExecSET(&e,0);
		m_instructionTime += 8;
	break;

	case 0xC4:
		//CBC4	 	SET 0,H			8	2	2
		ExecSET(&h,0);
		m_instructionTime += 8;
	break;

	case 0xC5:
		//CBC5	 	SET 0,L			8	2	2
		ExecSET(&l,0);
		m_instructionTime += 8;
	break;

	case 0xC6:
		//CBC6	 	SET 0,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSET(&n,0);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xC7:
		//CBC7	 	SET 0,A			8	2	2
		ExecSET(&a,0);
		m_instructionTime += 8;
	break;

	case 0xC8:
		//CBC8		SET 1,B			8	2	2
		ExecSET(&b,1);
		m_instructionTime += 8;
	break;

	case 0xC9:
		//CBC9	 	SET 1,C			8	2	2
		ExecSET(&c,1);
		m_instructionTime += 8;
	break;

	case 0xCA:
		//CBCA	 	SET 1,D			8	2	2
		ExecSET(&d,1);
		m_instructionTime += 8;
	break;

	case 0xCB:
		//CBCB	 	SET 1,E			8	2	2
		ExecSET(&e,1);
		m_instructionTime += 8;
	break;

	case 0xCC:
		//CBCC	 	SET 1,H			8	2	2
		ExecSET(&h,1);
		m_instructionTime += 8;
	break;

	case 0xCD:
		//CBCD	 	SET 1,L			8	2	2
		ExecSET(&l,1);
		m_instructionTime += 8;
	break;

	case 0xCE:
		//CBCE	 	SET 1,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSET(&n,1);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xCF:
		//CBCF	 	SET 1,A			8	2	2
		ExecSET(&a,1);
		m_instructionTime += 8;
	break;

	case 0xD0:
		//CBD0		SET 2,B			8	2	2
		ExecSET(&b,2);
		m_instructionTime += 8;
	break;

	case 0xD1:
		//CBD1	 	SET 2,C			8	2	2
		ExecSET(&c,2);
		m_instructionTime += 8;
	break;

	case 0xD2:
		//CBD2	 	SET 2,D			8	2	2
		ExecSET(&d,2);
		m_instructionTime += 8;
	break;

	case 0xD3:
		//CBD3	 	SET 2,E			8	2	2
		ExecSET(&e,2);
		m_instructionTime += 8;
	break;

	case 0xD4:
		//CBD4	 	SET 2,H			8	2	2
		ExecSET(&h,2);
		m_instructionTime += 8;
	break;

	case 0xD5:
		//CBD5	 	SET 2,L			8	2	2
		ExecSET(&l,2);
		m_instructionTime += 8;
	break;

	case 0xD6:
		//CBD6	 	SET 2,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSET(&n,2);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xD7:
		//CBD7	 	SET 2,A			8	2	2
		ExecSET(&a,2);
		m_instructionTime += 8;
	break;

	case 0xD8:
		//CBD8		SET 3,B			8	2	2
		ExecSET(&b,3);
		m_instructionTime += 8;
	break;

	case 0xD9:
		//CBD9	 	SET 3,C			8	2	2
		ExecSET(&c,3);
		m_instructionTime += 8;
	break;

	case 0xDA:
		//CBDA	 	SET 3,D			8	2	2
		ExecSET(&d,3);
		m_instructionTime += 8;
	break;

	case 0xDB:
		//CBDB	 	SET 3,E			8	2	2
		ExecSET(&e,3);
		m_instructionTime += 8;
	break;

	case 0xDC:
		//CBDC	 	SET 3,H			8	2	2
		ExecSET(&h,3);
		m_instructionTime += 8;
	break;

	case 0xDD:
		//CBDD	 	SET 3,L			8	2	2
		ExecSET(&l,3);
		m_instructionTime += 8;
	break;

	case 0xDE:
		//CBDE	 	SET 3,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSET(&n,3);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xDF:
		//CBDF	 	SET 3,A			8	2	2
		ExecSET(&a,3);
		m_instructionTime += 8;
	break;

	case 0xE0:
		//CBE0		SET 4,B			8	2	2
		ExecSET(&b,4);
		m_instructionTime += 8;
	break;

	case 0xE1:
		//CBE1	 	SET 4,C			8	2	2
		ExecSET(&c,4);
		m_instructionTime += 8;
	break;

	case 0xE2:
		//CBE2	 	SET 4,D			8	2	2
		ExecSET(&d,4);
		m_instructionTime += 8;
	break;

	case 0xE3:
		//CBE3	 	SET 4,E			8	2	2
		ExecSET(&e,4);
		m_instructionTime += 8;
	break;

	case 0xE4:
		//CBE4	 	SET 4,H			8	2	2
		ExecSET(&h,4);
		m_instructionTime += 8;
	break;

	case 0xE5:
		//CBE5	 	SET 4,L			8	2	2
		ExecSET(&l,4);
		m_instructionTime += 8;
	break;

	case 0xE6:
		//CBE6	 	SET 4,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSET(&n,4);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xE7:
		//CBE7	 	SET 4,A			8	2	2
		ExecSET(&a,4);
		m_instructionTime += 8;
	break;

	case 0xE8:
		//CBE8		SET 5,B			8	2	2
		ExecSET(&b,5);
		m_instructionTime += 8;
	break;

	case 0xE9:
		//CBE9	 	SET 5,C			8	2	2
		ExecSET(&c,5);
		m_instructionTime += 8;
	break;

	case 0xEA:
		//CBEA	 	SET 5,D			8	2	2
		ExecSET(&d,5);
		m_instructionTime += 8;
	break;

	case 0xEB:
		//CBEB	 	SET 5,E			8	2	2
		ExecSET(&e,5);
		m_instructionTime += 8;
	break;

	case 0xEC:
		//CBEC	 	SET 5,H			8	2	2
		ExecSET(&h,5);
		m_instructionTime += 8;
	break;

	case 0xED:
		//CBED	 	SET 5,L			8	2	2
		ExecSET(&l,5);
		m_instructionTime += 8;
	break;

	case 0xEE:
		//CBEE	 	SET 5,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSET(&n,5);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xEF:
		//CBEF	 	SET 5,A			8	2	2
		ExecSET(&a,5);
		m_instructionTime += 8;
	break;

	case 0xF0:
		//CBF0		SET 6,B			8	2	2
		ExecSET(&b,6);
		m_instructionTime += 8;
	break;

	case 0xF1:
		//CBF1	 	SET 6,C			8	2	2
		ExecSET(&c,6);
		m_instructionTime += 8;
	break;

	case 0xF2:
		//CBF2	 	SET 6,D			8	2	2
		ExecSET(&d,6);
		m_instructionTime += 8;
	break;

	case 0xF3:
		//CBF3	 	SET 6,E			8	2	2
		ExecSET(&e,6);
		m_instructionTime += 8;
	break;

	case 0xF4:
		//CBF4	 	SET 6,H			8	2	2
		ExecSET(&h,6);
		m_instructionTime += 8;
	break;

	case 0xF5:
		//CBF5	 	SET 6,L			8	2	2
		ExecSET(&l,6);
		m_instructionTime += 8;
	break;

	case 0xF6:
		//CBF6	 	SET 6,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSET(&n,6);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xF7:
		//CBF7	 	SET 6,A			8	2	2
		ExecSET(&a,6);
		m_instructionTime += 8;
	break;

	case 0xF8:
		//CBF8		SET 7,B			8	2	2
		ExecSET(&b,7);
		m_instructionTime += 8;
	break;

	case 0xF9:
		//CBF9	 	SET 7,C			8	2	2
		ExecSET(&c,7);
		m_instructionTime += 8;
	break;

	case 0xFA:
		//CBFA	 	SET 7,D			8	2	2
		ExecSET(&d,7);
		m_instructionTime += 8;
	break;

	case 0xFB:
		//CBFB	 	SET 7,E			8	2	2
		ExecSET(&e,7);
		m_instructionTime += 8;
	break;

	case 0xFC:
		//CBFC	 	SET 7,H			8	2	2
		ExecSET(&h,7);
		m_instructionTime += 8;
	break;

	case 0xFD:
		//CBFD	 	SET 7,L			8	2	2
		ExecSET(&l,7);
		m_instructionTime += 8;
	break;

	case 0xFE:
		//CBFE	 	SET 7,(HL)		15	4	2
		n = m_memory->Read8(hl);
		ExecSET(&n,7);
		m_memory->Write8(hl, n);
		m_instructionTime += 15;
	break;

	case 0xFF:
		//CBFF	 	SET 7,A			8	2	2
		ExecSET(&a,7);
		m_instructionTime += 8;
	break;

	default:
	break;
	}

	return m_instructionTime;
}
