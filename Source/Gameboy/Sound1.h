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
#ifndef SOUND1_H
#define SOUND1_H

#include "PlatformTypes.h"
class Gameboy;

#include "Sound.h"	///<for SquareSynthesisMethod
#include "SoundGenerator.h"
class DutyUnit;


class Sound1 : public SoundGenerator
{
public:

	Sound1();
	~Sound1();


	//Sound component

	virtual void Initialize(ChannelController* channelController);
	void SetMachine(Gameboy* machine);

	void SetSynthesisMethod(SquareSynthesisMethod::Type method);


	//Sound generation

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	void TickEnvelope();
	virtual void TickSweep();

	virtual float GetSample();


	//Registers

	void SetNR10(u8 value);
	void SetNR11(u8 value);
	void SetNR12(u8 value);
	void SetNR13(u8 value);
	void SetNR14(u8 value);


private:


	//Sound generation

	DutyUnit* m_dutyUnit;

	int m_frequency;	///<11-bit frequency


	virtual void Trigger();
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



	//Registers

	u8 m_nr10;	///<ff10
	u8 m_nr11;	///<ff11
	u8 m_nr12;	///<ff12
	u8 m_nr13;	///<ff13
	u8 m_nr14;	///<ff14
};

#endif
