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
#include "sound1.h"
using namespace emunisce;

#include "channel_controller.h"
#include "duty_unit.h"
#include "envelope_unit.h"
#include "gameboy_includes.h"
#include "length_unit.h"
#include "serialization/serialization_includes.h"

Sound1::Sound1() {
	m_machine = nullptr;

	m_dutyUnit = new DutyUnit();
	m_envelopeUnit = new EnvelopeUnit(this);

	m_lengthUnit->SetMaxValue(64);
}

Sound1::~Sound1() {
	delete m_envelopeUnit;
	delete m_dutyUnit;
}

// Sound component

void Sound1::Initialize(ChannelController* channelController) {
	SoundGenerator::Initialize(channelController);

	m_frequency = 0;
	m_frequencyShadow = 0;
	m_hasPerformedDecreasingCalculation = false;

	SetNR10(0x80);
	SetNR11(0x3f);
	SetNR12(0x00);
	SetNR13(0xff);
	SetNR14(0xbf);
}

void Sound1::SetMachine(Gameboy* machine) {
	SoundGenerator::SetMachine(machine);

	Memory* memory = machine->GetGbMemory();

	memory->SetRegisterLocation(0x10, &m_nr10, false);
	memory->SetRegisterLocation(0x11, &m_nr11, false);
	memory->SetRegisterLocation(0x12, &m_nr12, false);
	memory->SetRegisterLocation(0x13, &m_nr13, false);
	memory->SetRegisterLocation(0x14, &m_nr14, false);
}

void Sound1::Serialize(Archive& archive) {
	SoundGenerator::Serialize(archive);

	m_dutyUnit->Serialize(archive);

	SerializeItem(archive, m_frequency);

	SerializeItem(archive, m_frequencyShadow);

	SerializeItem(archive, m_sweepEnabled);
	SerializeItem(archive, m_sweepShift);
	SerializeItem(archive, m_sweepIncreasing);
	SerializeItem(archive, m_sweepTimerValue);
	SerializeItem(archive, m_sweepTimerPeriod);
	SerializeItem(archive, m_hasPerformedDecreasingCalculation);

	// Registers

	SerializeItem(archive, m_nr10);
	SerializeItem(archive, m_nr11);
	SerializeItem(archive, m_nr12);
	SerializeItem(archive, m_nr13);
	SerializeItem(archive, m_nr14);
}

void Sound1::SetSynthesisMethod(SquareSynthesisMethod::Type method) {
	m_dutyUnit->SetSynthesisMethod(method);
}

// Sound generation
void Sound1::PowerOff() {
	SetNR10(0);
	SetNR11(0);
	SetNR12(0);
	SetNR13(0);
	SetNR14(0);

	SoundGenerator::PowerOff();
}

void Sound1::PowerOn() {
	SoundGenerator::PowerOn();
}

void Sound1::Run(int ticks) {
	SoundGenerator::Run(ticks);

	m_dutyUnit->SetFrequency(m_frequency);
	m_dutyUnit->Run(ticks);
}

void Sound1::TickEnvelope() {
	m_envelopeUnit->Tick();
}

void Sound1::TickSweep() {
	if (m_sweepEnabled == false) {
		return;
	}

	m_sweepTimerValue--;
	if (m_sweepTimerValue > 0) {
		return;
	}

	if (m_sweepTimerPeriod == 0) {
		m_sweepTimerValue += 8;
	}
	else {
		m_sweepTimerValue += m_sweepTimerPeriod;
	}

	if (m_sweepTimerPeriod == 0) {
		return;
	}

	int newFrequency = CalculateFrequency();
	if (newFrequency > 2047) {
		m_channelController->DisableChannel();
		return;
	}

	if (m_sweepIncreasing == false) {
		m_hasPerformedDecreasingCalculation = true;
	}

	if (m_sweepShift == 0) {
		return;
	}

	if (newFrequency < 0) {
		newFrequency = 0;
	}

	m_frequency = newFrequency;
	m_frequencyShadow = newFrequency;

	newFrequency = CalculateFrequency();
	if (newFrequency > 2047) {
		m_channelController->DisableChannel();
		return;
	}
}

float Sound1::GetSample() {
	float sample = m_dutyUnit->GetSample();

	sample *= m_envelopeUnit->GetCurrentVolume();

	return sample;
}

// Registers

void Sound1::SetNR10(u8 value) {
	if (m_hasPower == false) {
		return;
	}

	WriteSweepRegister(value);

	m_nr10 = value & 0x7f;
	m_nr10 |= 0x80;
}

void Sound1::SetNR11(u8 value) {
	// DMG allows writing length even when the power is off
	// todo: CGB does not

	if (m_hasPower == true) {
		m_dutyUnit->WriteDutyRegister(value & 0xc0);
		m_nr11 = value & 0xc0;
	}

	m_lengthUnit->WriteLengthRegister(value & 0x3f);

	m_nr11 |= 0x3f;
}

void Sound1::SetNR12(u8 value) {
	if (m_hasPower == false) {
		return;
	}

	m_envelopeUnit->WriteEnvelopeRegister(value);

	m_nr12 = value;
}

void Sound1::SetNR13(u8 value) {
	if (m_hasPower == false) {
		return;
	}

	m_frequency &= ~(0xff);
	m_frequency |= value;

	m_nr13 = 0xff;
}

void Sound1::SetNR14(u8 value) {
	if (m_hasPower == false) {
		return;
	}

	m_frequency &= ~(0x700);
	m_frequency |= ((value & 0x07) << 8);

	WriteTriggerRegister(value);

	m_nr14 = value & 0x40;
	m_nr14 |= 0xbf;
}

void Sound1::Trigger() {
	SoundGenerator::Trigger();
	m_dutyUnit->Trigger();
	m_envelopeUnit->Trigger();
	TriggerSweep();
}

void Sound1::TriggerSweep() {
	m_hasPerformedDecreasingCalculation = false;

	m_frequencyShadow = m_frequency;

	m_sweepTimerValue = m_sweepTimerPeriod;
	if (m_sweepTimerPeriod == 0) {
		m_sweepTimerValue = 8;
	}

	if (m_sweepTimerPeriod == 0 && m_sweepShift == 0) {
		m_sweepEnabled = false;
	}
	else {
		m_sweepEnabled = true;
	}

	if (m_sweepShift > 0) {
		int newFrequency = CalculateFrequency();
		if (newFrequency > 2047) {
			m_channelController->DisableChannel();
		}

		if (m_sweepIncreasing == false) {
			m_hasPerformedDecreasingCalculation = true;
		}
	}
}

void Sound1::WriteSweepRegister(u8 value) {
	m_sweepShift = value & 0x07;

	if (value & 0x08) {
		m_sweepIncreasing = false;
	}
	else {
		m_sweepIncreasing = true;

		if (m_hasPerformedDecreasingCalculation) {
			m_channelController->DisableChannel();
		}
	}

	m_sweepTimerPeriod = (value & 0x70) >> 4;
}

int Sound1::CalculateFrequency() {
	int delta = m_frequencyShadow >> m_sweepShift;

	int result = m_frequencyShadow;

	if (m_sweepIncreasing == true) {
		result += delta;
	}
	else {
		result -= delta;
	}

	return result;
}
