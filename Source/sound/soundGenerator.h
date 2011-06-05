#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H

#include "../common/types.h"

class ChannelDisabler;


class SoundGenerator
{
public:

	SoundGenerator();

	virtual void Initialize(ChannelDisabler* channelDisabler);

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	virtual void TickLength();
	//TickDuty?  Maybe put that in Sound2 and have Sound1 inherit from Sound2?
	virtual void TickEnvelope();

	virtual float GetSample();

protected:

	//WriteDutyRegister?  Maybe put that in Sound2 and have Sound1 inherit from Sound2?
	void WriteEnvelopeRegister(u8 value);

	bool m_hasPower;
	ChannelDisabler* m_channelDisabler;


	//Length counter

	void WriteLengthRegister(u8 value);
	bool m_lengthCounterEnabled;
	int m_lengthCounterValue;
	int m_lengthCounterMaxValue;
};

#endif
