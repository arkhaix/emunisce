#ifndef SOUND4_H
#define SOUND4_H

#include "../common/types.h"
class Machine;

#include "soundGenerator.h"


class Sound4 : public SoundGenerator
{
public:

	Sound4();
	~Sound4();


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

	void SetNR41(u8 value);
	void SetNR42(u8 value);
	void SetNR43(u8 value);
	void SetNR44(u8 value);


private:

	virtual void Trigger();


	//Sound generation

	u16 m_lfsr;
	int m_lfsrTapBit;
	int m_lfsrFeedbackBit;
	float m_lfsrOut;

	int m_timerPeriod;
	int m_timerValue;


	//Registers

	u8 m_nr40;	///<ff1f
	u8 m_nr41;	///<ff20
	u8 m_nr42;	///<ff21
	u8 m_nr43;	///<ff22
	u8 m_nr44;	///<ff23
};

#endif
