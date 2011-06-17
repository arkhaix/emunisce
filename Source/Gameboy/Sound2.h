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
#ifndef SOUND2_H
#define SOUND2_H

#include "PlatformTypes.h"

#include "Sound.h"	///<for SquareSynthesisMethod
#include "SoundGenerator.h"



namespace Emunisce
{

class Gameboy;
class DutyUnit;


class Sound2 : public SoundGenerator
{
public:

	Sound2();
	~Sound2();


	//Sound component

	virtual void Initialize(ChannelController* channelController);
	void SetMachine(Gameboy* machine);

	void SetSynthesisMethod(SquareSynthesisMethod::Type method);


	//Sound generation

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	void TickEnvelope();

	virtual float GetSample();


	//Registers

	void SetNR21(u8 value);
	void SetNR22(u8 value);
	void SetNR23(u8 value);
	void SetNR24(u8 value);


private:

	virtual void Trigger();


	//Sound generation

	DutyUnit* m_dutyUnit;
	int m_frequency;


	//Registers

	u8 m_nr20;	///<ff15
	u8 m_nr21;	///<ff16
	u8 m_nr22;	///<ff17
	u8 m_nr23;	///<ff18
	u8 m_nr24;	///<ff19
};

}	//namespace Emunisce

#endif
