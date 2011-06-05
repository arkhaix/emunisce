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

	bool m_envelopeEnabled;
	bool m_envelopeVolumeIncreasing;
	int m_envelopeInitialVolume;
	int m_envelopeVolume;
	int m_envelopeTimer;
	int m_envelopePeriod;
};

#endif
