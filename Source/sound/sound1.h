#ifndef SOUND1_H
#define SOUND1_H

#include "../common/types.h"

class Machine;

class Sound1
{
public:

	Sound1();


	//Sound component

	void Initialize();
	void SetMachine(Machine* machine);


	//Sound generation

	void PowerOff();
	void PowerOn();

	void Run(int ticks);
	float GetSample();


	//Registers

	void SetNR10(u8 value);
	void SetNR11(u8 value);
	void SetNR12(u8 value);
	void SetNR13(u8 value);
	void SetNR14(u8 value);


private:

	//Sound component

	Machine* m_machine;
	bool m_hasPower;


	//Registers

	u8 m_nr10;	///<ff10
	u8 m_nr11;	///<ff11
	u8 m_nr12;	///<ff12
	u8 m_nr13;	///<ff13
	u8 m_nr14;	///<ff14
};

#endif