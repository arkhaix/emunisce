#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H

#include "../common/types.h"
class Machine;

class ChannelController;


class SoundGenerator
{
public:

	SoundGenerator();
	~SoundGenerator();

	virtual void Initialize(ChannelController* channelController);
	virtual void SetMachine(Machine* machine);

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	virtual void TickLength();

	virtual float GetSample();

protected:

	virtual void Trigger();
	virtual void WriteTriggerRegister(u8 value);

	Machine* m_machine;
	bool m_hasPower;
	bool m_dacEnabled;
	ChannelController* m_channelController;


	//Length counter

	friend class LengthUnit;
	LengthUnit* m_lengthUnit;


	//Sweep

	//Duty

	//Envelope

	friend class EnvelopeUnit;
	EnvelopeUnit* m_envelopeUnit;

	//Noise
};

#endif
