/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

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
#include "Sound4.h"
using namespace Emunisce;

#include "GameboyIncludes.h"

#include "EnvelopeUnit.h"
#include "LengthUnit.h"


Sound4::Sound4()
{
	m_machine = NULL;

	m_envelopeUnit = new EnvelopeUnit(this);

	m_lengthUnit->SetMaxValue(64);

	m_lfsr = 0xffff;
	m_lfsrTapBit = 7;
	m_lfsrFeedbackBit = 15;
	m_lfsrOut = 0.f;

	m_timerPeriod = 0;
	m_timerValue = 0;
}

Sound4::~Sound4()
{
	delete m_envelopeUnit;
}


//Sound component

void Sound4::Initialize(ChannelController* channelController)
{
	SoundGenerator::Initialize(channelController);

	m_nr40 = 0xff;	///<inaccessible
	SetNR41(0xff);
	SetNR42(0xff);
	SetNR43(0x00);
	SetNR44(0xbf);
}

void Sound4::SetMachine(Gameboy* machine)
{
	SoundGenerator::SetMachine(machine);

	Memory* memory = machine->GetGbMemory();

	memory->SetRegisterLocation(0x1f, &m_nr40, false);
	memory->SetRegisterLocation(0x20, &m_nr41, false);
	memory->SetRegisterLocation(0x21, &m_nr42, false);
	memory->SetRegisterLocation(0x22, &m_nr43, false);
	memory->SetRegisterLocation(0x23, &m_nr44, false);
}


//Sound generation

void Sound4::PowerOff()
{
	m_lfsr = 0xffff;
	m_lfsrOut = 0.f;

	SetNR41(0);
	SetNR42(0);
	SetNR43(0);
	SetNR44(0);

	SoundGenerator::PowerOff();
}

void Sound4::PowerOn()
{
	SoundGenerator::PowerOn();
}


void Sound4::Run(int ticks)
{
	SoundGenerator::Run(ticks);

	if(m_timerPeriod == 0)
		return;

	m_timerValue -= ticks;
	while(m_timerValue <= 0)
	{
		m_timerValue += m_timerPeriod;

		int a = (m_lfsr & 1) ? 1 : 0;
		int b = (m_lfsr & (1<<m_lfsrTapBit)) ? 1 : 0;

		m_lfsr >>= 1;

		int result = a ^ b;
		if(result)
		{
			m_lfsr |= (1<<m_lfsrFeedbackBit);
			m_lfsrOut = -1.f;
		}
		else
		{
			m_lfsr &= ~(1<<m_lfsrFeedbackBit);
			m_lfsrOut = 1.f;
		}
	}
}


void Sound4::TickEnvelope()
{
	m_envelopeUnit->Tick();
}


float Sound4::GetSample()
{
	float sample = m_lfsrOut;
	
	sample *= m_envelopeUnit->GetCurrentVolume();

	return sample;
}


//Registers

void Sound4::SetNR41(u8 value)
{
	//DMG allows writing this even when the power is off
	//todo: CGB does not

	m_lengthUnit->WriteLengthRegister(value & 0x3f);

	m_nr41 = 0xff;
}

void Sound4::SetNR42(u8 value)
{
	if(m_hasPower == false)
		return;

	m_envelopeUnit->WriteEnvelopeRegister(value);

	m_nr42 = value;
}

void Sound4::SetNR43(u8 value)
{
	if(m_hasPower == false)
		return;

	if(value & 0x08)
	{
		m_lfsrFeedbackBit = 7;
		m_lfsrTapBit = 1;	///<1 is most tone-like, 3 is most white-noise-like
	}
	else
	{
		m_lfsrFeedbackBit = 15;
		m_lfsrTapBit = 7;
	}

	int divisor = (value & 0x07);
	if(divisor == 0)
		divisor = 8;
	else
		divisor = 16 * divisor;

	int divisorShift = (value & 0xf0) >> 4;
	divisorShift++;

	m_timerPeriod = divisor << divisorShift;

	m_nr43 = value;
}

void Sound4::SetNR44(u8 value)
{
	if(m_hasPower == false)
		return;

	WriteTriggerRegister(value);

	m_nr44 = value & 0x40;
	m_nr44 |= 0xbf;
}


void Sound4::Trigger()
{
	m_lfsr = 0xffff;
	m_timerValue = m_timerPeriod;

	SoundGenerator::Trigger();
	m_envelopeUnit->Trigger();
}
