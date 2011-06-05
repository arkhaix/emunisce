#ifndef SOUND3_H
#define SOUND3_H

#include "../common/types.h"

class Machine;

class Sound3
{
public:

	Sound3();


	//Sound component

	void Initialize();
	void SetMachine(Machine* machine);


	//Sound generation

	void PowerOff();
	void PowerOn();

	void Run(int ticks);
	float GetSample();


	//Registers

	void SetNR30(u8 value);
	void SetNR31(u8 value);
	void SetNR32(u8 value);
	void SetNR33(u8 value);
	void SetNR34(u8 value);


private:

	//Sound component

	Machine* m_machine;


	//Registers

	u8 m_nr30;	///<ff1a
	u8 m_nr31;	///<ff1b
	u8 m_nr32;	///<ff1c
	u8 m_nr33;	///<ff1d
	u8 m_nr34;	///<ff1e
};

#endif
