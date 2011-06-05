#ifndef ENVELOPEUNIT_H
#define ENVELOPEUNIT_H

#include "../common/types.h"

class SoundGenerator;

class EnvelopeUnit
{
public:

	EnvelopeUnit(SoundGenerator* soundGenerator);

	void Tick();

	void WriteEnvelopeRegister(u8 value);


private:

	SoundGenerator* m_soundGenerator;

	bool m_enabled;

	bool m_volumeIncreasing;
	int m_initialVolume;
	int m_currentVolume;
	
	int m_timerValue;
	int m_timerPeriod;
};

#endif
