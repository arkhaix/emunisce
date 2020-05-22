/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef INPUT_H
#define INPUT_H

#include "gameboy_types.h"
#include "MachineIncludes.h"
#include "PlatformTypes.h"

namespace emunisce {

namespace GameboyButtons {
typedef int Type;

enum {
	Right = 0,
	Left,
	Up,
	Down,

	A,
	B,
	Select,
	Start,

	NumGameboyButtons
};

#ifdef GameboyButtons_ToString
static const char* ToString[] = {"Right",
								 "Left",
								 "Up",
								 "Down",

								 "A",
								 "B",
								 "Select",
								 "Start",

								 "NumGameboyButtons"};
#endif

}  // namespace GameboyButtons

namespace RegisterMode {
typedef int Type;

enum {
	MachineType = 0,
	P14,
	P15,

	NumModes
};
}  // namespace RegisterMode

class Input : public EmulatedInput {
public:
	Input();
	virtual ~Input() = default;

	// IEmulatedInput

	unsigned int NumButtons() override;
	const char* GetButtonName(unsigned int index) override;

	void ButtonDown(unsigned int index) override;
	void ButtonUp(unsigned int index) override;

	bool IsButtonDown(unsigned int index) override;

	// Input

	// Component
	void SetMachine(Gameboy* machine);
	void Initialize();

	virtual void Serialize(Archive& archive);

	// Registers
	void SetJoypadMode(u8 value);

private:
	void UpdateRegister();
	void UpdateInterruptFlag();

	Gameboy* m_machine;

	RegisterMode::Type m_currentMode;

	u8 m_buttonStates;

	u8 m_joypadRegister;
};

}  // namespace emunisce

#endif
