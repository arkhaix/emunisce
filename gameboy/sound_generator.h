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
#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H

#include "PlatformTypes.h"

namespace emunisce {

class Archive;

class Gameboy;
class ChannelController;
class EnvelopeUnit;
class LengthUnit;

class SoundGenerator {
public:
	SoundGenerator();
	~SoundGenerator();

	virtual void Initialize(ChannelController* channelController);
	virtual void SetMachine(Gameboy* machine);

	virtual void Serialize(Archive& archive);

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	virtual void TickLength();

	virtual float GetSample();

protected:
	virtual void Trigger();
	virtual void WriteTriggerRegister(u8 value);

	Gameboy* m_machine;
	bool m_hasPower;
	bool m_dacEnabled;
	ChannelController* m_channelController;

	// Length counter

	friend class LengthUnit;
	LengthUnit* m_lengthUnit;

	// Sweep

	// Duty

	// Envelope

	friend class EnvelopeUnit;
	EnvelopeUnit* m_envelopeUnit;

	// Noise
};

}  // namespace emunisce

#endif
