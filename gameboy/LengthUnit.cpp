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
#include "LengthUnit.h"
using namespace Emunisce;

#include "ChannelController.h"
#include "GameboyIncludes.h"
#include "Serialization/SerializationIncludes.h"
#include "Sound.h"
#include "SoundGenerator.h"

LengthUnit::LengthUnit(SoundGenerator* soundGenerator) {
	m_soundGenerator = soundGenerator;
}

void LengthUnit::Serialize(Archive& archive) {
	SerializeItem(archive, m_enabled);

	SerializeItem(archive, m_value);
	SerializeItem(archive, m_maxValue);
}

void LengthUnit::SetMaxValue(int maxValue) {
	m_maxValue = maxValue;
}

void LengthUnit::Tick() {
	if (m_enabled == false) {
		return;
	}

	if (m_value > 0) {
		m_value--;

		if (m_value == 0) {
			m_soundGenerator->m_channelController->DisableChannel();
		}
	}
}

void LengthUnit::Trigger() {
	if (m_value == 0) {
		m_value = m_maxValue;
	}

	int frameSequencerPosition = m_soundGenerator->m_machine->GetGbSound()->GetFrameSequencerPosition();

	if (frameSequencerPosition == 0 || frameSequencerPosition == 2 || frameSequencerPosition == 4 ||
		frameSequencerPosition == 6) {
		if (m_enabled == true && m_value == m_maxValue) {
			m_value--;
		}
	}
}

void LengthUnit::Enable() {
	int frameSequencerPosition = m_soundGenerator->m_machine->GetGbSound()->GetFrameSequencerPosition();

	if (frameSequencerPosition == 0 || frameSequencerPosition == 2 || frameSequencerPosition == 4 ||
		frameSequencerPosition == 6) {
		if (m_enabled == false && m_value > 0) {
			m_value--;

			if (m_value == 0) {
				m_soundGenerator->m_channelController->DisableChannel();
			}
		}
	}

	m_enabled = true;
}

void LengthUnit::Disable() {
	m_enabled = false;
}

void LengthUnit::WriteLengthRegister(u8 value) {
	m_value = m_maxValue - value;
}
