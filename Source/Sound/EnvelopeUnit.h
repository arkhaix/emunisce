#ifndef ENVELOPEUNIT_H
#define ENVELOPEUNIT_H

#include "../Common/Types.h"

class SoundGenerator;

class EnvelopeUnit
{
public:

	EnvelopeUnit(SoundGenerator* soundGenerator);

	void Tick();
	void Trigger();

	void WriteEnvelopeRegister(u8 value);

	float GetCurrentVolume();


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
