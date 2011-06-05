#ifndef SOUND2_H
#define SOUND2_H

#include "../common/types.h"
class Machine;

#include "soundGenerator.h"


class Sound2 : public SoundGenerator
{
public:

	Sound2();


	//Sound component

	virtual void Initialize();
	void SetMachine(Machine* machine);


	//Sound generation

	virtual void PowerOff();
	virtual void PowerOn();

	virtual void Run(int ticks);
	virtual float GetSample();


	//Registers

	void SetNR21(u8 value);
	void SetNR22(u8 value);
	void SetNR23(u8 value);
	void SetNR24(u8 value);


private:

	//Sound component

	Machine* m_machine;


	//Registers

	u8 m_nr20;	///<ff15
	u8 m_nr21;	///<ff16
	u8 m_nr22;	///<ff17
	u8 m_nr23;	///<ff18
	u8 m_nr24;	///<ff19
};

#endif
