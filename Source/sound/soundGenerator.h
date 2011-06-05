#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H

#include "../common/types.h"
class Machine;

class ChannelController;


class SoundGenerator
{
public:

	SoundGenerator();

	virtual void Initialize(ChannelController* channelController);
	virtual void SetMachine(Machine* machine);

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	virtual void TickLength();
	//TickDuty?  Maybe put that in Sound2 and have Sound1 inherit from Sound2?
	virtual void TickEnvelope();

	virtual float GetSample();

protected:

	//WriteDutyRegister?  Maybe put that in Sound2 and have Sound1 inherit from Sound2?

	virtual void Trigger();
	virtual void WriteTriggerRegister(u8 value);

	Machine* m_machine;
	bool m_hasPower;
	ChannelController* m_channelController;


	//Length counter

	void EnableLengthCounter();
	void DisableLengthCounter();
	void WriteLengthRegister(u8 value);

	bool m_lengthCounterEnabled;
	int m_lengthCounterValue;
	int m_lengthCounterMaxValue;


	//Envelope

	void WriteEnvelopeRegister(u8 value);
};

#endif
