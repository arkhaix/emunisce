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
#include "DutyUnit.h"
using namespace Emunisce;

#include "serialization/SerializationIncludes.h"

DutyUnit::DutyUnit() {
	m_timerPeriod = 0;
	m_timerValue = 0;

	m_dutyPosition = 0;
	m_dutyMode = 0;

	int dutyTable[4][8] = {
		{0, 0, 0, 0, 0, 0, 0, 1}, {1, 0, 0, 0, 0, 0, 0, 1}, {1, 0, 0, 0, 0, 1, 1, 1}, {0, 1, 1, 1, 1, 1, 1, 0}};

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 8; j++) {
			m_dutyTable[i][j] = dutyTable[i][j];
		}
	}

	m_synthesisMethod = SquareSynthesisMethod::LinearInterpolation;
	m_hasTransitioned = false;
	m_hitNyquist = false;
	m_ticksSinceLastSample = 0;
	m_sumSinceLastSample = 0;
}

void DutyUnit::Serialize(Archive& archive) {
	SerializeItem(archive, m_timerPeriod);
	SerializeItem(archive, m_timerValue);

	SerializeItem(archive, m_dutyPosition);
	SerializeItem(archive, m_dutyMode);

	SerializeItem(archive, m_synthesisMethod);

	SerializeItem(archive, m_hasTransitioned);
	SerializeItem(archive, m_hitNyquist);
	SerializeItem(archive, m_ticksSinceLastSample);
	SerializeItem(archive, m_sumSinceLastSample);
}

void DutyUnit::Run(int ticks) {
	if (m_timerPeriod == 0) {
		return;
	}

	if (m_synthesisMethod == SquareSynthesisMethod::Naive) {
		m_timerValue -= ticks;
		while (m_timerValue <= 0) {
			m_timerValue += m_timerPeriod;

			m_dutyPosition++;
			if (m_dutyPosition > 7) {
				m_dutyPosition = 0;
			}
		}

		return;
	}

	else if (m_synthesisMethod == SquareSynthesisMethod::LinearInterpolation) {
		if (m_hitNyquist == true) {
			return;
		}

		m_ticksSinceLastSample += ticks;

		if (m_timerValue > ticks) {
			m_timerValue -= ticks;

			int sampleValue = 1;
			if (m_dutyTable[m_dutyMode][m_dutyPosition] == 0) {
				sampleValue = -1;
			}

			m_sumSinceLastSample += sampleValue * ticks;
			return;
		}

		if (m_hasTransitioned == true) {
			m_hitNyquist = true;
			return;
		}

		m_hasTransitioned = true;

		int sampleValue = 1;
		if (m_dutyTable[m_dutyMode][m_dutyPosition] == 0) {
			sampleValue = -1;
		}

		m_sumSinceLastSample += sampleValue * m_timerValue;

		m_dutyPosition++;
		if (m_dutyPosition > 7) {
			m_dutyPosition = 0;
		}

		ticks -= m_timerValue;
		m_timerValue = m_timerPeriod;

		return Run(ticks);
	}

	// else

	return;
}

void DutyUnit::Trigger() {
	m_dutyPosition = 0;
	m_timerValue = m_timerPeriod;
}

void DutyUnit::SetFrequency(int frequency) {
	m_timerPeriod = (2048 - frequency) * 4;
}

void DutyUnit::WriteDutyRegister(u8 value) {
	m_dutyMode = (value & 0xc0) >> 6;
}

float DutyUnit::GetSample() {
	if (m_synthesisMethod == SquareSynthesisMethod::Naive) {
		if (m_dutyTable[m_dutyMode][m_dutyPosition] == 0) {
			return -1.f;
		}

		return 1.f;
	}

	else if (m_synthesisMethod == SquareSynthesisMethod::LinearInterpolation) {
		float result = 0.f;

		if (m_ticksSinceLastSample > 0 && m_hitNyquist == false) {
			result = (float)m_sumSinceLastSample / (float)m_ticksSinceLastSample;
		}

		m_hasTransitioned = false;
		m_hitNyquist = false;
		m_ticksSinceLastSample = 0;
		m_sumSinceLastSample = 0;

		return result;
	}

	// else

	return 0.f;
}

void DutyUnit::SetSynthesisMethod(SquareSynthesisMethod::Type method) {
	m_synthesisMethod = method;

	if (method < 0 || method >= SquareSynthesisMethod::NumSquareSynthesisMethods) {
		m_synthesisMethod = SquareSynthesisMethod::LinearInterpolation;
	}
}
