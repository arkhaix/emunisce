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
	//TickDuty?  Maybe put that in Sound2 and have Sound1 inherit from Sound2?
	virtual void TickEnvelope();

	virtual float GetSample();

protected:

	//WriteDutyRegister?  Maybe put that in Sound2 and have Sound1 inherit from Sound2?

	virtual void Trigger();
	virtual void WriteTriggerRegister(u8 value);

	Machine* m_machine;
	bool m_hasPower;
	bool m_dacEnabled;
	ChannelController* m_channelController;


	//Length counter

	friend class LengthUnit;
	LengthUnit* m_lengthUnit;


	//Envelope

	void WriteEnvelopeRegister(u8 value);

	bool m_envelopeEnabled;
	bool m_envelopeVolumeIncreasing;
	int m_envelopeInitialVolume;
	int m_envelopeVolume;
	int m_envelopeTimer;
	int m_envelopePeriod;
};

#endif
