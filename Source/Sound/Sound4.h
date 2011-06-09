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
#ifndef SOUND4_H
#define SOUND4_H

#include "../Common/Types.h"
class Machine;

#include "SoundGenerator.h"


class Sound4 : public SoundGenerator
{
public:

	Sound4();
	~Sound4();


	//Sound component

	virtual void Initialize(ChannelController* channelController);
	void SetMachine(Machine* machine);


	//Sound generation

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	void TickEnvelope();

	virtual float GetSample();


	//Registers

	void SetNR41(u8 value);
	void SetNR42(u8 value);
	void SetNR43(u8 value);
	void SetNR44(u8 value);


private:

	virtual void Trigger();


	//Sound generation

	u16 m_lfsr;
	int m_lfsrTapBit;
	int m_lfsrFeedbackBit;
	float m_lfsrOut;

	int m_timerPeriod;
	int m_timerValue;


	//Registers

	u8 m_nr40;	///<ff1f
	u8 m_nr41;	///<ff20
	u8 m_nr42;	///<ff21
	u8 m_nr43;	///<ff22
	u8 m_nr44;	///<ff23
};

#endif
