#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H

#include "../common/types.h"


class SoundGenerator
{
public:

	SoundGenerator();

	virtual void Initialize();

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	virtual void TickLength();
	//TickDuty?  Maybe put that in Sound2 and have Sound1 inherit from Sound2?
	virtual void TickEnvelope();

	virtual float GetSample();

protected:

	void WriteLengthRegister(u8 value);
	//WriteDutyRegister?  Maybe put that in Sound2 and have Sound1 inherit from Sound2?
	void WriteEnvelopeRegister(u8 value);

	bool m_hasPower;
};

#endif
