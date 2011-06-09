/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Sound3.h"

#include "../Common/Machine.h"
#include "../Memory/Memory.h"

#include "ChannelController.h"
#include "LengthUnit.h"


Sound3::Sound3()
{
	m_machine = NULL;

	m_lengthUnit->SetMaxValue(256);

	m_frequency = 0;
	m_outputLevelShift = 0;

	m_waveTimerPeriod = 0;
	m_waveTimerValue = 0;

	m_waveSamplePosition = 0;
}


//Sound component

void Sound3::Initialize(ChannelController* channelController)
{
	SoundGenerator::Initialize(channelController);

	SetNR30(0x7f);
	SetNR31(0xff);
	SetNR32(0x9f);
	SetNR33(0xff);
	SetNR34(0xbf);
}

void Sound3::SetMachine(Machine* machine)
{
	SoundGenerator::SetMachine(machine);

	Memory* memory = machine->GetMemory();

	memory->SetRegisterLocation(0x1a, &m_nr30, false);
	memory->SetRegisterLocation(0x1b, &m_nr31, false);
	memory->SetRegisterLocation(0x1c, &m_nr32, false);
	memory->SetRegisterLocation(0x1d, &m_nr33, false);
	memory->SetRegisterLocation(0x1e, &m_nr34, false);

	memory->SetWaveRamLock(WaveRamLock::Normal);
	for(u16 waveRamAddress = 0xff30; waveRamAddress <= 0xff3f; waveRamAddress++)
		memory->Write8(waveRamAddress, (waveRamAddress & 1) ? 0xff : 0x00);
}


//Sound generation

void Sound3::PowerOff()
{
	m_waveSamplePosition = 0;

	SetNR30(0);
	SetNR31(0);
	SetNR32(0);
	SetNR33(0);
	SetNR34(0);

	SoundGenerator::PowerOff();
}

void Sound3::PowerOn()
{
	SoundGenerator::PowerOn();
}


void Sound3::Run(int ticks)
{
	SoundGenerator::Run(ticks);

	if(m_waveTimerPeriod == 0 || m_channelController->IsChannelEnabled() == false)
	{
		m_machine->GetMemory()->SetWaveRamLock(WaveRamLock::Normal);
		return;
	}

	bool adjustedReadTimer = false;

	m_waveTimerValue -= ticks;
	while(m_waveTimerValue <= 0)
	{
		m_waveSamplePosition++;
		if(m_waveSamplePosition > 31)
			m_waveSamplePosition = 0;

		int waveSampleIndex = m_waveSamplePosition / 2;
		u16 waveSampleAddress = (u16)0xff30 + waveSampleIndex;

		m_machine->GetMemory()->SetWaveRamLock(WaveRamLock::Normal);

		m_waveSampleValue = m_machine->GetMemory()->Read8(waveSampleAddress);

		m_machine->GetMemory()->SetWaveRamLock(WaveRamLock::SingleValue, m_waveSampleValue);

		//Which tick did the access happen on?
		int ticksSinceMemoryAccess = -m_waveTimerValue;
		m_sampleReadTimerValue = 2 - ticksSinceMemoryAccess;	///<The constant here is a wild guess
		adjustedReadTimer = true;

		m_waveTimerValue += m_waveTimerPeriod;	///<Normally, this would be at the top of the while loop.  It's down here for memoryAccessTick simplification.
	}

	if(adjustedReadTimer == false && m_sampleReadTimerValue > 0)
	{
		m_sampleReadTimerValue -= ticks;
		if(m_sampleReadTimerValue <= 0)
			m_machine->GetMemory()->SetWaveRamLock(WaveRamLock::NoAccess);
	}
	else if(m_sampleReadTimerValue <= 0)
	{
		m_machine->GetMemory()->SetWaveRamLock(WaveRamLock::NoAccess);
	}
}

float Sound3::GetSample()
{
	u8 waveSampleValue = m_waveSampleValue;	///<Copied locally because of the >>= later.

	//Samples at even positions are in the high nibble, samples at odd positions are in the low nibble
	if(m_waveSamplePosition & 1)
		waveSampleValue &= 0x0f;
	else
		waveSampleValue = (waveSampleValue & 0xf0) >> 4;

	//Volume
	waveSampleValue >>= m_outputLevelShift;

	//Convert from [0x00,0x0f] integer range to [0,1] float range
	float sample = (float)waveSampleValue / (float)0x0f;

	//Convert from [0,1] range to [-1,+1] range
	sample *= 2.f;
	sample -= 1.f;

	return sample;
}


//Registers

void Sound3::SetNR30(u8 value)
{
	if(m_hasPower == false)
		return;

	//Disable the DAC?
	if((value & 0x80) == 0)
	{
		m_channelController->DisableChannel();
		m_dacEnabled = false;
		m_machine->GetMemory()->SetWaveRamLock(WaveRamLock::Normal);
	}
	else
	{
		m_dacEnabled = true;
		m_machine->GetMemory()->SetWaveRamLock(WaveRamLock::NoAccess);
	}

	m_nr30 = value & 0x80;
	m_nr30 |= 0x7f;
}

void Sound3::SetNR31(u8 value)
{
	//DMG allows writing this even when the power is off
	//todo: CGB does not

	m_lengthUnit->WriteLengthRegister(value);

	m_nr31 = 0xff;
}

void Sound3::SetNR32(u8 value)
{
	if(m_hasPower == false)
		return;

	m_outputLevelShift = (value & 0x60) >> 5;
	if(m_outputLevelShift == 0)
		m_outputLevelShift = 4;
	else
		m_outputLevelShift -= 1;

	m_nr32 = value & 0x60;
	m_nr32 |= 0x9f;
}

void Sound3::SetNR33(u8 value)
{
	if(m_hasPower == false)
		return;

	m_frequency &= ~(0xff);
	m_frequency |= value;

	m_waveTimerPeriod = (2048 - m_frequency) * 2;

	m_nr33 = 0xff;
}

void Sound3::SetNR34(u8 value)
{
	if(m_hasPower == false)
		return;

	m_frequency &= ~(0x700);
	m_frequency |= ((value & 0x07) << 8);

	m_waveTimerPeriod = (2048 - m_frequency) * 2;

	WriteTriggerRegister(value);

	m_nr34 = value & 0x40;
	m_nr34 |= 0xbf;
}


void Sound3::Trigger()
{
	m_sampleReadTimerValue = 0;
	m_waveTimerValue = m_waveTimerPeriod;
	m_waveSamplePosition = 31;	///<Auto-increments.  The first sample read should be the 0th one.

	SoundGenerator::Trigger();
}
