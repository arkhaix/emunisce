#ifndef SOUND4_H
#define SOUND4_H

#include "../common/types.h"

class Machine;

class Sound4
{
public:

	Sound4();


	//Sound component

	void Initialize();
	void SetMachine(Machine* machine);


	//Sound generation

	void PowerOff();
	void PowerOn();

	void Run(int ticks);
	float GetSample();


	//Registers

	void SetNR41(u8 value);
	void SetNR42(u8 value);
	void SetNR43(u8 value);
	void SetNR44(u8 value);


private:

	//Sound component

	Machine* m_machine;


	//Registers

	u8 m_nr40;	///<ff1f
	u8 m_nr41;	///<ff20
	u8 m_nr42;	///<ff21
	u8 m_nr43;	///<ff22
	u8 m_nr44;	///<ff23

};

#endif
