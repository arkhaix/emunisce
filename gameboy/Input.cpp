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
#define GameboyButtons_ToString 1

#include "Input.h"
using namespace emunisce;

#include "GameboyIncludes.h"
#include "serialization/SerializationIncludes.h"

Input::Input() {
}

// Component
void Input::SetMachine(Gameboy* machine) {
	m_machine = machine;
	machine->GetGbMemory()->SetRegisterLocation(0x00, &m_joypadRegister, false);
}

void Input::Initialize() {
	m_buttonStates = 0xff;

	m_currentMode = RegisterMode::MachineType;
	m_joypadRegister = 0xff;
}

void Input::Serialize(Archive& archive) {
	SerializeItem(archive, m_currentMode);
	SerializeItem(archive, m_buttonStates);
	SerializeItem(archive, m_joypadRegister);
}

// External

unsigned int Input::NumButtons() {
	return GameboyButtons::NumGameboyButtons;
}

const char* Input::GetButtonName(unsigned int index) {
	if (index >= GameboyButtons::NumGameboyButtons) {
		return nullptr;
	}

	return GameboyButtons::ToString[index];
}

void Input::ButtonDown(unsigned int index) {
	u8 oldButtonStates = m_buttonStates;

	m_buttonStates &= ~(1 << index);

	if (m_buttonStates == oldButtonStates) {
		return;
	}

	if (m_currentMode == RegisterMode::P14 || m_currentMode == RegisterMode::P15) {
		UpdateRegister();
	}

	UpdateInterruptFlag();
}

void Input::ButtonUp(unsigned int index) {
	u8 oldButtonStates = m_buttonStates;

	m_buttonStates |= 1 << index;

	if (m_buttonStates == oldButtonStates) {
		return;
	}

	if (m_currentMode == RegisterMode::P14 || m_currentMode == RegisterMode::P15) {
		UpdateRegister();
	}

	UpdateInterruptFlag();
}

bool Input::IsButtonDown(unsigned int index) {
	if (m_buttonStates & (1 << index)) {
		return false;
	}

	return true;
}

// Registers
void Input::SetJoypadMode(u8 value) {
	if ((value & 0x10) == 0) {
		m_currentMode = RegisterMode::P14;
	}
	else if ((value & 0x20) == 0) {
		m_currentMode = RegisterMode::P15;
	}
	else {
		m_currentMode = RegisterMode::MachineType;
	}

	UpdateRegister();
}

void Input::UpdateRegister() {
	if (m_currentMode == RegisterMode::P14) {
		m_joypadRegister = m_buttonStates & 0x0f;
	}
	else if (m_currentMode == RegisterMode::P15) {
		m_joypadRegister = (m_buttonStates >> 4) & 0x0f;
	}
	else  //(m_currentMode == RegisterMode::MachineType)
	{
		m_joypadRegister = 0xff;
	}
}

void Input::UpdateInterruptFlag() {
	// Nothing pressed, so clear the interrupt flag
	if (m_buttonStates == 0xff) {
		u8 interrupts = m_machine->GetGbMemory()->Read8(REG_IF);
		interrupts &= ~(IF_INPUT);
		m_machine->GetGbMemory()->Write8(REG_IF, interrupts);
	}

	// Something's pressed, so set the interrupt flag
	else {
		u8 interrupts = m_machine->GetGbMemory()->Read8(REG_IF);
		interrupts |= IF_INPUT;
		m_machine->GetGbMemory()->Write8(REG_IF, interrupts);
	}
}
