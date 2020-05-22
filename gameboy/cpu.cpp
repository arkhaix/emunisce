/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "cpu.h"
using namespace emunisce;

#include "GameboyIncludes.h"
#include "serialization/SerializationIncludes.h"

Cpu::Cpu()
	: a(*(((u8*)&af) + 1)),
	  f(*(((u8*)&af) + 0)),
	  b(*(((u8*)&bc) + 1)),
	  c(*(((u8*)&bc) + 0)),
	  d(*(((u8*)&de) + 1)),
	  e(*(((u8*)&de) + 0)),
	  h(*(((u8*)&hl) + 1)),
	  l(*(((u8*)&hl) + 0)) {
	m_machine = nullptr;
	m_memory = nullptr;
}

void Cpu::SetMachine(Gameboy* machine) {
	m_machine = machine;
	m_machineType = machine->GetType();

	m_memory = machine->GetGbMemory();

	m_memory->SetRegisterLocation(0x0f, &m_interruptFlags, true);
	m_memory->SetRegisterLocation(0xff, &m_interruptsEnabled, true);

	m_memory->SetRegisterLocation(0x04, &m_timerDivider, false);
	m_memory->SetRegisterLocation(0x05, &m_timerCounter, true);
	m_memory->SetRegisterLocation(0x06, &m_timerModulo, true);
	m_memory->SetRegisterLocation(0x07, &m_timerControl, false);

	m_memory->SetRegisterLocation(0x4d, &m_cgbSpeedSwitch, false);
}

void Cpu::Initialize() {
	m_masterInterruptsEnabled = false;
	m_delayNextInterrupt = false;

	m_interruptsEnabled = 0;
	m_interruptFlags = 0;

	m_interruptsEnabled =
		0;  ///< 0xffff - Interrupt Enable.  Which interrupts are currently enabled.  Slaves to the IME flag.
	m_interruptFlags = 0;  ///< 0xff0f - Interrupt Flag.  Which interrupts are currently set.

	m_cgbSpeedSwitch = 0;  ///< ff4d - Speed switch and flag (Key1).  Whether speed switching is enabled (0x01) and
						   ///< whether double-speed is currently active (0x80).

	m_timerDivider = 0;                ///< 0xff04 - Timer Divider.
	m_ticksPerDividerIncrement = 256;  ///< The timer divider increments once every 256 ticks.
	m_ticksUntilDividerIncrement = m_ticksPerDividerIncrement;

	m_timerModulo = 0;  ///< 0xff06 - Timer Modulo.  This value gets loaded into the timer counter when it overflows.

	m_timerCounter = 0;  ///< 0xff05 - Timer Counter.

	SetTimerControl(0);
	m_ticksUntilCounterIncrement = m_ticksPerCounterIncrement;

	m_halted = false;
	m_stopped = false;
	m_haltBug = false;

	af = 0x01b0;
	bc = 0x0013;
	de = 0x00d8;
	hl = 0x014d;

	sp = 0xfffe;

	pc = 0x0000;

	if (m_machineType == EmulatedMachine::GameboyColor) {
		a = 0x11;
	}
}

bool Cpu::IsStopped() {
	return m_stopped;
}

void Cpu::Serialize(Archive& archive) {
	SerializeItem(archive, pc);
	SerializeItem(archive, sp);

	SerializeItem(archive, af);
	SerializeItem(archive, bc);
	SerializeItem(archive, de);
	SerializeItem(archive, hl);

	SerializeItem(archive, m_masterInterruptsEnabled);
	SerializeItem(archive, m_delayNextInterrupt);

	SerializeItem(archive, m_halted);
	SerializeItem(archive, m_stopped);
	SerializeItem(archive, m_haltBug);

	// Registers
	SerializeItem(archive, m_interruptsEnabled);
	SerializeItem(archive, m_interruptFlags);

	SerializeItem(archive, m_timerDivider);
	SerializeItem(archive, m_ticksPerDividerIncrement);
	SerializeItem(archive, m_ticksUntilDividerIncrement);

	SerializeItem(archive, m_timerModulo);

	SerializeItem(archive, m_timerCounter);
	SerializeItem(archive, m_ticksPerCounterIncrement);
	SerializeItem(archive, m_ticksUntilCounterIncrement);

	SerializeItem(archive, m_timerControl);
	SerializeItem(archive, m_timerEnabled);

	SerializeItem(archive, m_cgbSpeedSwitch);
}

void Cpu::SetTimerDivider(u8 value) {
	m_timerDivider = 0;
}

void Cpu::SetTimerControl(u8 value) {
	m_timerControl = value & 0x07;

	// TAC[1:0] Input Clock Select
	int clockSelect = value & 0x03;

	if (clockSelect == 0) {
		m_ticksPerCounterIncrement = 1024;  ///< 4096 Hz
	}
	else if (clockSelect == 1) {
		m_ticksPerCounterIncrement = 16;  ///< 262144 Hz
	}
	else if (clockSelect == 2) {
		m_ticksPerCounterIncrement = 64;  ///< 65536 Hz
	}
	else {                                 //(clockSelect == 3)
		m_ticksPerCounterIncrement = 256;  ///< 16384 Hz
	}

	m_ticksUntilCounterIncrement = m_ticksPerCounterIncrement;

	// TAC[2] Timer Enable
	m_timerEnabled = false;
	if (value & 0x04) {
		m_timerEnabled = true;
	}

	RunTimer(0);
}

void Cpu::SetCgbSpeedSwitch(u8 value) {
	if (m_machineType != EmulatedMachine::GameboyColor) {
		return;
	}

	// Only bit 0 (speed switch enable) is writable
	m_cgbSpeedSwitch &= ~(0x01);
	m_cgbSpeedSwitch |= (value & 0x01);
}

void Cpu::RunTimer(int ticks) {
	// Divider
	m_ticksUntilDividerIncrement -= ticks;
	if (m_ticksUntilDividerIncrement <= 0) {
		m_ticksUntilDividerIncrement += m_ticksPerDividerIncrement;

		m_timerDivider++;
	}

	if (m_timerEnabled == false) {
		return;
	}

	// Counter
	m_ticksUntilCounterIncrement -= ticks;
	while (m_ticksUntilCounterIncrement <= 0) {
		m_ticksUntilCounterIncrement += m_ticksPerCounterIncrement;

		m_timerCounter++;
		if (m_timerCounter == 0) {
			m_timerCounter = m_timerModulo;

			// interrupt on overflow
			u8 interrupts = m_memory->Read8(REG_IF);
			interrupts |= IF_TIMER;
			m_memory->Write8(REG_IF, interrupts);
		}
	}
}

u8 Cpu::ReadNext8() {
	u8 result = m_memory->Read8(pc);
	pc++;
	return result;
}

u16 Cpu::ReadNext16() {
	u16 result = m_memory->Read16(pc);
	pc += 2;
	return result;
}

//////////////////////////////////////////////////////////////////

void Cpu::ExecADC(u8* target, u8 value) {
	int res = *target + value + TST_C;

	// Z
	if ((res & 0xff) == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	if ((*target ^ value ^ res) & 0x10) {
		SET_H;
	}
	else {
		RES_H;
	}

	// C
	if (res & 0x100) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target = (u8)res;
}

void Cpu::ExecADC(u16* target, u16 value) {
	int res = *target + value + TST_C;

	// Z
	if ((res & 0xffff) == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	if ((*target ^ value ^ res) & 0x1000) {
		SET_H;
	}
	else {
		RES_H;
	}

	// C
	if (res & 0x10000) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target = (u16)res;
}

void Cpu::ExecADD(u8* target, u8 value) {
	int res = *target + value;

	// Z
	if ((res & 0xff) == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	if ((*target ^ value ^ res) & 0x10) {
		SET_H;
	}
	else {
		RES_H;
	}

	// C
	if (res & 0x100) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target = (u8)res;
}

void Cpu::ExecADD(u16* target, u16 value) {
	int res = *target + value;

	// Z unaffected

	// N
	RES_N;

	// H
	if ((*target ^ value ^ res) & 0x1000) {
		SET_H;
	}
	else {
		RES_H;
	}

	// C
	if (res & 0x10000) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target = (u16)res;
}

void Cpu::ExecADD(u16* target, s8 value) {
	int res = *target + value;

	// H,C handled the same as ADD8
	u8 tempA = (u8)(*target);
	u8 tempB = (u8)value;
	ExecADD(&tempA, tempB);

	// Z
	RES_Z;

	// N
	RES_N;

	*target = (u16)res;
}

void Cpu::ExecAND(u8 value) {
	a = a & value;

	// Z
	if (a == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	SET_H;

	// C
	RES_C;
}

void Cpu::ExecBIT(u8 value, int n) {
	// Z
	if (value & (1 << n)) {
		RES_Z;
	}
	else {
		SET_Z;
	}

	// N
	RES_N;

	// H
	SET_H;

	// C unaffected
}

void Cpu::ExecCALL(u16 address) {
	//(SP-1)<-PCH
	sp--;
	m_memory->Write8(sp, (u8)((pc & 0xff00) >> 8));

	//(SP-2)<-PCL
	sp--;
	m_memory->Write8(sp, (u8)(pc & 0x00ff));

	// PC<-nn
	pc = address;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecCCF() {
	// Z unaffected

	// N
	RES_N;

	// H
	RES_H;

	// C
	INV_C;
}

void Cpu::ExecCP(u8 value) {
	int res = a - value;

	// Z
	if (res == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	SET_N;

	// H
	//???
	if ((a ^ value ^ res) & 0x10) {
		SET_H;
	}
	else {
		RES_H;
	}

	// C
	//???
	if (res < 0) {
		SET_C;
	}
	else {
		RES_C;
	}
}

void Cpu::ExecCPL() {
	a ^= 0xff;

	// Z unaffected

	// N
	SET_N;

	// H
	SET_H;

	// C unaffected
}

void Cpu::ExecDAA() {
	if (TST_N) {
		if (TST_C && TST_H) {
			a += 0x9a;
		}
		else if (TST_C) {
			a += 0xa0;
		}
		else if (TST_H) {
			a += 0xfa;
		}
	}
	else {
		if (a >= 0x9a) {
			a += 0x60;
			SET_C;

			if (TST_H || ((a & 0x0f) >= 0x0a)) {
				a += 0x06;
			}
		}
		else {
			if (TST_C) {
				a += 0x60;
			}
			if (TST_H || ((a & 0x0f) >= 0x0a)) {
				a += 0x06;
			}
		}
	}

	// Z
	if (a == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N unaffected

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecDEC(u8* target) {
	u8 res = (*target) - 1;

	// Z
	if (res == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	SET_N;

	// H
	if ((*target & 0x0f) == 0) {
		SET_H;
	}
	else {
		RES_H;
	}

	// C unaffected

	*target = res;
}

void Cpu::ExecDEC(u16* target) {
	*target = (*target) - 1;

	//???
	// Are these really unaffected?  Doesn't make sense.

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecDI() {
	m_masterInterruptsEnabled = false;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecEI() {
	m_masterInterruptsEnabled = true;
	m_delayNextInterrupt = true;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecEX(u16* target1, u16* target2) {
	u16 tmp = *target1;
	*target1 = *target2;
	*target2 = tmp;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecHALT() {
	m_halted = true;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecINC(u8* target) {
	u8 res = *target + 1;

	// Z
	if (res == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	if ((*target & 0x0f) == 0x0f) {
		SET_H;
	}
	else {
		RES_H;
	}

	// C unaffected

	*target = res;
}

void Cpu::ExecINC(u16* target) {
	*target = (*target) + 1;

	//???
	// Are these really unaffected?  Doesn't make sense.

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecJP(u16 address) {
	pc = address;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecJR(s8 value) {
	pc += value;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecLD(u8* target, u8 value) {
	*target = value;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecLD(u16* target, u16 value) {
	*target = value;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecNOP() {
	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecOR(u8* target) {
	a |= *target;

	// Z
	if (a == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C
	RES_C;
}

void Cpu::ExecOR(u8 value) {
	a |= value;

	// Z
	if (a == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C
	RES_C;
}

void Cpu::ExecPOP(u16* target) {
	// L<-(SP)
	*target = m_memory->Read8(sp);

	// H<-(SP+1)
	sp++;
	*target |= (m_memory->Read8(sp) << 8);

	sp++;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecPUSH(u16* target) {
	//(SP-1)<-H
	sp--;
	m_memory->Write8(sp, (u8)(*target >> 8));

	//(SP-2)<-L
	sp--;
	m_memory->Write8(sp, (u8)(*target & 0x00ff));

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecRES(u8* target, int n) {
	*target &= ~(1 << n);

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecRET() {
	// PCL<-(SP)
	pc = m_memory->Read8(sp);

	// PCH<-(SP+1)
	sp++;
	pc |= (m_memory->Read8(sp) << 8);

	sp++;

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecRL(u8* target) {
	int oldC = TST_C;

	if (*target & 0x80) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target <<= 1;
	*target |= oldC;

	// Z
	if (*target == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecRLA() {
	int oldC = TST_C;

	if (a & 0x80) {
		SET_C;
	}
	else {
		RES_C;
	}

	a <<= 1;
	a |= oldC;

	// Z
	RES_Z;

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecRLC(u8* target) {
	if (*target & 0x80) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target <<= 1;
	*target |= TST_C;

	// Z
	if (*target == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecRLCA() {
	if (a & 0x80) {
		SET_C;
	}
	else {
		RES_C;
	}

	a <<= 1;
	a |= TST_C;

	// Z
	RES_Z;

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecRR(u8* target) {
	int oldC = TST_C;

	if (*target & 0x01) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target >>= 1;
	*target |= (oldC << 7);

	// Z
	if (*target == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecRRA() {
	int oldC = TST_C;

	if (a & 0x01) {
		SET_C;
	}
	else {
		RES_C;
	}

	a >>= 1;
	a |= (oldC << 7);

	// Z
	RES_Z;

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecRRC(u8* target) {
	if (*target & 0x01) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target >>= 1;
	*target |= (TST_C << 7);

	// Z
	if (*target == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecRRCA() {
	if (a & 0x01) {
		SET_C;
	}
	else {
		RES_C;
	}

	a >>= 1;
	a |= (TST_C << 7);

	// Z
	RES_Z;

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecRST(u16 address) {
	ExecCALL(address);
}

void Cpu::ExecSBC(u8 value) {
	int res = a - value - TST_C;

	// Z
	if ((res & 0xff) == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	SET_N;

	// H
	if ((a ^ value ^ res) & 0x10) {
		SET_H;
	}
	else {
		RES_H;
	}

	// C
	if (res < 0) {
		SET_C;
	}
	else {
		RES_C;
	}

	a = (u8)res;
}

void Cpu::ExecSCF() {
	// Z unaffected

	// N
	RES_N;

	// H
	RES_H;

	// C
	SET_C;
}

void Cpu::ExecSET(u8* target, int n) {
	*target |= (1 << n);

	// Z unaffected

	// N unaffected

	// H unaffected

	// C unaffected
}

void Cpu::ExecSLA(u8* target) {
	if (*target & 0x80) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target <<= 1;

	// Z
	if (*target == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecSRA(u8* target) {
	int bit7 = (*target & 0x80);

	if (*target & 0x01) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target >>= 1;
	*target |= bit7;

	// Z
	if (*target == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecSRL(u8* target) {
	if (*target & 0x01) {
		SET_C;
	}
	else {
		RES_C;
	}

	*target >>= 1;

	// Z
	if (*target == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C handled above
}

void Cpu::ExecSUB(u8 value) {
	int res = a - value;

	// Z
	if (res == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	SET_N;

	// H
	if ((a ^ value ^ res) & 0x10) {
		SET_H;
	}
	else {
		RES_H;
	}

	// C
	if (res < 0) {
		SET_C;
	}
	else {
		RES_C;
	}

	a = (u8)res;
}

void Cpu::ExecSWAP(u8* target) {
	u8 low = (*target) & 0x0f;
	*target >>= 4;
	*target |= (low << 4);

	// Z
	if (*target == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C
	RES_C;
}

void Cpu::ExecXOR(u8 value) {
	a ^= value;

	// Z
	if (a == 0) {
		SET_Z;
	}
	else {
		RES_Z;
	}

	// N
	RES_N;

	// H
	RES_H;

	// C
	RES_C;
}
