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
#ifndef SOUND1_H
#define SOUND1_H

#include "PlatformTypes.h"
#include "Sound.h"  ///<for SquareSynthesisMethod
#include "SoundGenerator.h"

namespace emunisce {

class Gameboy;
class DutyUnit;

class Sound1 : public SoundGenerator {
public:
	Sound1();
	virtual ~Sound1();

	// Sound component

	void Initialize(ChannelController* channelController) override;
	void SetMachine(Gameboy* machine) override;

	void Serialize(Archive& archive) override;

	void SetSynthesisMethod(SquareSynthesisMethod::Type method);

	// Sound generation

	void PowerOff() override;
	void PowerOn() override;

	void Run(int ticks) override;

	void TickEnvelope();
	virtual void TickSweep();

	float GetSample() override;

	// Registers

	void SetNR10(u8 value);
	void SetNR11(u8 value);
	void SetNR12(u8 value);
	void SetNR13(u8 value);
	void SetNR14(u8 value);

private:
	// Sound generation

	DutyUnit* m_dutyUnit;

	int m_frequency;  ///< 11-bit frequency

	void Trigger() override;
	void TriggerSweep();
	void WriteSweepRegister(u8 value);

	int CalculateFrequency();

	int m_frequencyShadow;

	bool m_sweepEnabled;
	int m_sweepShift;
	bool m_sweepIncreasing;
	int m_sweepTimerValue;
	int m_sweepTimerPeriod;
	bool m_hasPerformedDecreasingCalculation;

	// Registers

	u8 m_nr10;  ///< ff10
	u8 m_nr11;  ///< ff11
	u8 m_nr12;  ///< ff12
	u8 m_nr13;  ///< ff13
	u8 m_nr14;  ///< ff14
};

}  // namespace emunisce

#endif
