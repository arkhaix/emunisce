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
#ifndef SOUND3_H
#define SOUND3_H

#include "PlatformTypes.h"
#include "sound_generator.h"

namespace emunisce {

class Gameboy;
class ChannelController;

class Sound3 : public SoundGenerator {
public:
	Sound3();
	virtual ~Sound3() = default;

	// Sound component

	void Initialize(ChannelController* channelController) override;
	void SetMachine(Gameboy* machine) override;

	void Serialize(Archive& archive) override;

	// Sound generation

	void PowerOff() override;
	void PowerOn() override;

	void Run(int ticks) override;
	float GetSample() override;

	// Registers

	void SetNR30(u8 value);
	void SetNR31(u8 value);
	void SetNR32(u8 value);
	void SetNR33(u8 value);
	void SetNR34(u8 value);

private:
	void Trigger() override;

	// Sound generation

	int m_frequency;
	int m_outputLevelShift;

	int m_waveTimerPeriod;
	int m_waveTimerValue;

	int m_waveSamplePosition;

	u8 m_waveSampleValue;

	// Memory access

	int m_sampleReadTimerValue;

	// Registers

	u8 m_nr30;  ///< ff1a
	u8 m_nr31;  ///< ff1b
	u8 m_nr32;  ///< ff1c
	u8 m_nr33;  ///< ff1d
	u8 m_nr34;  ///< ff1e
};

}  // namespace emunisce

#endif
