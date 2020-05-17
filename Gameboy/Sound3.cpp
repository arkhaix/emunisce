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
#include "Sound3.h"
using namespace Emunisce;

#include "ChannelController.h"
#include "GameboyIncludes.h"
#include "LengthUnit.h"
#include "Serialization/SerializationIncludes.h"

Sound3::Sound3() {
	m_machine = nullptr;

	m_lengthUnit->SetMaxValue(256);

	m_frequency = 0;
	m_outputLevelShift = 0;

	m_waveTimerPeriod = 0;
	m_waveTimerValue = 0;

	m_waveSamplePosition = 0;
}

// Sound component

void Sound3::Initialize(ChannelController* channelController) {
	SoundGenerator::Initialize(channelController);

	SetNR30(0x7f);
	SetNR31(0xff);
	SetNR32(0x9f);
	SetNR33(0xff);
	SetNR34(0xbf);
}

void Sound3::SetMachine(Gameboy* machine) {
	SoundGenerator::SetMachine(machine);

	Memory* memory = machine->GetGbMemory();

	memory->SetRegisterLocation(0x1a, &m_nr30, false);
	memory->SetRegisterLocation(0x1b, &m_nr31, false);
	memory->SetRegisterLocation(0x1c, &m_nr32, false);
	memory->SetRegisterLocation(0x1d, &m_nr33, false);
	memory->SetRegisterLocation(0x1e, &m_nr34, false);

	memory->SetWaveRamLock(WaveRamLock::Normal);
	for (u16 waveRamAddress = 0xff30; waveRamAddress <= 0xff3f; waveRamAddress++) {
		memory->Write8(waveRamAddress, (waveRamAddress & 1) ? 0xff : 0x00);
	}
}

void Sound3::Serialize(Archive& archive) {
	SoundGenerator::Serialize(archive);

	SerializeItem(archive, m_frequency);
	SerializeItem(archive, m_outputLevelShift);

	SerializeItem(archive, m_waveTimerPeriod);
	SerializeItem(archive, m_waveTimerValue);

	SerializeItem(archive, m_waveSamplePosition);

	SerializeItem(archive, m_waveSampleValue);

	// Memory access

	SerializeItem(archive, m_sampleReadTimerValue);

	// Registers

	SerializeItem(archive, m_nr30);
	SerializeItem(archive, m_nr31);
	SerializeItem(archive, m_nr32);
	SerializeItem(archive, m_nr33);
	SerializeItem(archive, m_nr34);
}

// Sound generation

void Sound3::PowerOff() {
	m_waveSamplePosition = 0;

	SetNR30(0);
	SetNR31(0);
	SetNR32(0);
	SetNR33(0);
	SetNR34(0);

	SoundGenerator::PowerOff();
}

void Sound3::PowerOn() {
	SoundGenerator::PowerOn();
}

void Sound3::Run(int ticks) {
	SoundGenerator::Run(ticks);

	if (m_waveTimerPeriod == 0 || m_channelController->IsChannelEnabled() == false) {
		m_machine->GetGbMemory()->SetWaveRamLock(WaveRamLock::Normal);
		return;
	}

	bool adjustedReadTimer = false;

	m_waveTimerValue -= ticks;
	while (m_waveTimerValue <= 0) {
		m_waveSamplePosition++;
		if (m_waveSamplePosition > 31) {
			m_waveSamplePosition = 0;
		}

		int waveSampleIndex = m_waveSamplePosition / 2;
		u16 waveSampleAddress = (u16)0xff30 + waveSampleIndex;

		m_machine->GetGbMemory()->SetWaveRamLock(WaveRamLock::Normal);

		m_waveSampleValue = m_machine->GetGbMemory()->Read8(waveSampleAddress);

		m_machine->GetGbMemory()->SetWaveRamLock(WaveRamLock::SingleValue, m_waveSampleValue);

		// Which tick did the access happen on?
		int ticksSinceMemoryAccess = -m_waveTimerValue;
		m_sampleReadTimerValue =
			2 -
			ticksSinceMemoryAccess;  ///< The constant here is just guesswork.  It represents how long (ticks) the data
									 ///< stays available after a read.  It seems that values of 1 and 2 both work.
		adjustedReadTimer = true;

		m_waveTimerValue += m_waveTimerPeriod;  ///< Normally, this would be at the top of the while loop.  It's down
												///< here for memoryAccessTick simplification.
	}

	if (adjustedReadTimer == false && m_sampleReadTimerValue > 0) {
		m_sampleReadTimerValue -= ticks;
		if (m_sampleReadTimerValue <= 0) {
			m_machine->GetGbMemory()->SetWaveRamLock(WaveRamLock::NoAccess);
		}
	}
	else if (m_sampleReadTimerValue <= 0) {
		m_machine->GetGbMemory()->SetWaveRamLock(WaveRamLock::NoAccess);
	}
}

float Sound3::GetSample() {
	u8 waveSampleValue = m_waveSampleValue;  ///< Copied locally because of the >>= later.

	// Samples at even positions are in the high nibble, samples at odd positions are in the low nibble
	if (m_waveSamplePosition & 1) {
		waveSampleValue &= 0x0f;
	}
	else {
		waveSampleValue = (waveSampleValue & 0xf0) >> 4;
	}

	// Volume
	waveSampleValue >>= m_outputLevelShift;

	// Convert from [0x00,0x0f] integer range to [0,1] float range
	float sample = (float)waveSampleValue / (float)0x0f;

	// Convert from [0,1] range to [-1,+1] range
	sample *= 2.f;
	sample -= 1.f;

	return sample;
}

// Registers

void Sound3::SetNR30(u8 value) {
	if (m_hasPower == false) {
		return;
	}

	// Disable the DAC?
	if ((value & 0x80) == 0) {
		m_channelController->DisableChannel();
		m_dacEnabled = false;
		m_machine->GetGbMemory()->SetWaveRamLock(WaveRamLock::Normal);
	}
	else {
		m_dacEnabled = true;
		m_machine->GetGbMemory()->SetWaveRamLock(WaveRamLock::NoAccess);
		m_waveTimerValue = m_waveTimerPeriod;
	}

	m_nr30 = value & 0x80;
	m_nr30 |= 0x7f;
}

void Sound3::SetNR31(u8 value) {
	// DMG allows writing this even when the power is off
	// todo: CGB does not

	m_lengthUnit->WriteLengthRegister(value);

	m_nr31 = 0xff;
}

void Sound3::SetNR32(u8 value) {
	if (m_hasPower == false) {
		return;
	}

	m_outputLevelShift = (value & 0x60) >> 5;
	if (m_outputLevelShift == 0) {
		m_outputLevelShift = 4;
	}
	else {
		m_outputLevelShift -= 1;
	}

	m_nr32 = value & 0x60;
	m_nr32 |= 0x9f;
}

void Sound3::SetNR33(u8 value) {
	if (m_hasPower == false) {
		return;
	}

	m_frequency &= ~(0xff);
	m_frequency |= value;

	m_waveTimerPeriod = (2048 - m_frequency) * 2;

	m_nr33 = 0xff;
}

void Sound3::SetNR34(u8 value) {
	if (m_hasPower == false) {
		return;
	}

	m_frequency &= ~(0x700);
	m_frequency |= ((value & 0x07) << 8);

	m_waveTimerPeriod = (2048 - m_frequency) * 2;

	WriteTriggerRegister(value);

	m_nr34 = value & 0x40;
	m_nr34 |= 0xbf;
}

void Sound3::Trigger() {
	// Wave ram gets corrupted if a trigger occurs during a read
	if (m_sampleReadTimerValue > 0 && m_dacEnabled == true) {
		m_machine->GetGbMemory()->SetWaveRamLock(WaveRamLock::Normal);

		u16 waveRamAddress = 0xff30;

		int nextWaveSamplePosition =
			m_waveSamplePosition + 1;  ///< Not sure if this variable is necessary.  It might just need to use
									   ///< m_waveSamplePosition with no offset.
		if (nextWaveSamplePosition > 31) {
			nextWaveSamplePosition -= 32;
		}

		// If it's reading one of the first four bytes, then the first byte gets overwritten
		if (nextWaveSamplePosition < 8) {
			u16 sampleAddress = waveRamAddress + (nextWaveSamplePosition / 2);  ///< 2 samples per byte

			u8 sampleByteValue = m_machine->GetGbMemory()->Read8(sampleAddress);

			m_machine->GetGbMemory()->Write8(waveRamAddress, sampleByteValue);
		}

		// Otherwise, the first four bytes get overwritten with the four aligned bytes currently being accessed
		else {
			u16 sampleAddress = waveRamAddress + (nextWaveSamplePosition / 2);  ///< 2 samples per byte
			u16 alignedSampleAddress =
				sampleAddress & 0xfff8;  ///< Align to 4-byte boundary = set the 3 lowest bits to 0

			for (int i = 0; i < 4; i++) {
				u8 newValue = m_machine->GetGbMemory()->Read8(alignedSampleAddress + i);
				m_machine->GetGbMemory()->Write8(waveRamAddress + i, newValue);
			}
		}

		// m_machine->GetGbMemory()->SetWaveRamLock(WaveRamLock::SingleValue, m_waveSampleValue);
	}

	m_sampleReadTimerValue = 0;
	m_waveSamplePosition = 0;

	m_waveTimerValue =
		m_waveTimerPeriod + 6;  ///< More guesswork here.  The constant represents a delay between triggering and actual
								///< playback start.  It seems that values of 5 and 6 both work.

	SoundGenerator::Trigger();
}
