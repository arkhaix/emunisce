#ifndef SOUND2_H
#define SOUND2_H

#include "../Common/Types.h"
class Machine;

#include "SoundGenerator.h"
class DutyUnit;


class Sound2 : public SoundGenerator
{
public:

	Sound2();
	~Sound2();


	//Sound component

	virtual void Initialize(ChannelController* channelController);
	void SetMachine(Machine* machine);


	//Sound generation

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	void TickEnvelope();

	virtual float GetSample();


	//Registers

	void SetNR21(u8 value);
	void SetNR22(u8 value);
	void SetNR23(u8 value);
	void SetNR24(u8 value);


private:

	virtual void Trigger();


	//Sound generation

	DutyUnit* m_dutyUnit;
	int m_frequency;


	//Registers

	u8 m_nr20;	///<ff15
	u8 m_nr21;	///<ff16
	u8 m_nr22;	///<ff17
	u8 m_nr23;	///<ff18
	u8 m_nr24;	///<ff19
};

#endif
