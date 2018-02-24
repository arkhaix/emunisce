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
#ifndef DUTYUNIT_H
#define DUTYUNIT_H

#include "Sound.h"	///<for SquareSynthesisMethod


namespace Emunisce
{

class DutyUnit
{
public:

	DutyUnit();

	void Serialize(Archive& archive);

	void Run(int ticks);
	void Trigger();

	void SetFrequency(int frequency);
	void WriteDutyRegister(u8 value);

	float GetSample();

	void SetSynthesisMethod(SquareSynthesisMethod::Type method);

private:

	int m_timerPeriod;
	int m_timerValue;

	int m_dutyPosition;
	int m_dutyMode;
	int m_dutyTable[4][8];

	SquareSynthesisMethod::Type m_synthesisMethod;

	bool m_hasTransitioned;
	bool m_hitNyquist;
	int m_ticksSinceLastSample;
	int m_sumSinceLastSample;
};

}	//namespace Emunisce

#endif
