#ifndef INPUT_H
#define INPUT_H

#include "../Common/Types.h"

namespace Buttons
{
	typedef int Type;

	enum
	{
		Right = 0,
		Left,
		Up,
		Down,

		A,
		B,
		Select,
		Start,

		NumButtons
	};

	static const char* ToString[] =
	{
		"Right",
		"Left",
		"Up",
		"Down",

		"A",
		"B",
		"Select",
		"Start",

		"NumButtons"
	};

}	//namespace Buttons

namespace RegisterMode
{
	typedef int Type;

	enum
	{
		MachineType = 0,
		P14,
		P15,

		NumModes
	};
}	//namespace RegisterMode

class Input
{
public:

	Input();

	//Component
	void SetMachine(Machine* machine);
	void Initialize();

	//External
	void ButtonDown(Buttons::Type button);
	void ButtonUp(Buttons::Type button);

	//Registers
	void SetJoypadMode(u8 value);

private:

	void UpdateRegister();
	void UpdateInterruptFlag();

	Machine* m_machine;

	RegisterMode::Type m_currentMode;

	u8 m_buttonStates;

	u8 m_joypadRegister;
};

#endif
