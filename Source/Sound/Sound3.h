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
#ifndef SOUND3_H
#define SOUND3_H

#include "../Common/Types.h"
class Machine;

#include "SoundGenerator.h"


class Sound3 : public SoundGenerator
{
public:

	Sound3();


	//Sound component

	virtual void Initialize(ChannelController* channelController);
	void SetMachine(Machine* machine);


	//Sound generation

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);
	virtual float GetSample();


	//Registers

	void SetNR30(u8 value);
	void SetNR31(u8 value);
	void SetNR32(u8 value);
	void SetNR33(u8 value);
	void SetNR34(u8 value);


private:

	virtual void Trigger();


	//Sound generation

	int m_frequency;
	int m_outputLevelShift;

	int m_waveTimerPeriod;
	int m_waveTimerValue;

	int m_waveSamplePosition;


	//Registers

	u8 m_nr30;	///<ff1a
	u8 m_nr31;	///<ff1b
	u8 m_nr32;	///<ff1c
	u8 m_nr33;	///<ff1d
	u8 m_nr34;	///<ff1e
};

#endif
