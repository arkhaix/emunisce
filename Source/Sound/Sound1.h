#ifndef SOUND1_H
#define SOUND1_H

#include "../Common/Types.h"
class Machine;

#include "SoundGenerator.h"
class DutyUnit;


class Sound1 : public SoundGenerator
{
public:

	Sound1();
	~Sound1();


	//Sound component

	virtual void Initialize(ChannelController* channelController);
	void SetMachine(Machine* machine);


	//Sound generation

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);

	void TickEnvelope();
	virtual void TickSweep();

	virtual float GetSample();


	//Registers

	void SetNR10(u8 value);
	void SetNR11(u8 value);
	void SetNR12(u8 value);
	void SetNR13(u8 value);
	void SetNR14(u8 value);


private:


	//Sound generation

	DutyUnit* m_dutyUnit;

	int m_frequency;	///<11-bit frequency


	virtual void Trigger();
	void TriggerSweep();
	void WriteSweepRegister(u8 value);

	int CalculateFrequency();

	int m_frequencyShadow;

	bool m_sweepEnabled;
	int m_sweepShift;
	bool m_sweepIncreasing;
	int m_sweepTimerValue;
	int m_sweepTimerPeriod;
	bool m_hasPerformedDecreasingCalculation;



	//Registers

	u8 m_nr10;	///<ff10
	u8 m_nr11;	///<ff11
	u8 m_nr12;	///<ff12
	u8 m_nr13;	///<ff13
	u8 m_nr14;	///<ff14
};

#endif
