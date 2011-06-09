/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
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
