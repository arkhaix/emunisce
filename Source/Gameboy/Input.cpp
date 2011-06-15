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
#include "Input.h"

#include "GameboyIncludes.h"

Input::Input()
{
}

//Component
void Input::SetMachine(Gameboy* machine)
{
	m_machine = machine;
	machine->GetGbMemory()->SetRegisterLocation(0x00, &m_joypadRegister, false);
}

void Input::Initialize()
{
	m_buttonStates = 0xff;

	m_currentMode = RegisterMode::MachineType;
	m_joypadRegister = 0xff;
}

//External
void Input::ButtonDown(Buttons::Type button)
{
	u8 oldButtonStates = m_buttonStates;
	
	m_buttonStates &= ~(1<<button);

	if(m_buttonStates == oldButtonStates)
		return;

	if(m_currentMode == RegisterMode::P14 || m_currentMode == RegisterMode::P15)
		UpdateRegister();

	UpdateInterruptFlag();
}

void Input::ButtonUp(Buttons::Type button)
{
	u8 oldButtonStates = m_buttonStates;

	m_buttonStates |= 1<<button;

	if(m_buttonStates == oldButtonStates)
		return;

	if(m_currentMode == RegisterMode::P14 || m_currentMode == RegisterMode::P15)
		UpdateRegister();

	UpdateInterruptFlag();
}

//Registers
void Input::SetJoypadMode(u8 value)
{
	if( (value & 0x10) == 0)
		m_currentMode = RegisterMode::P14;
	else if( (value & 0x20) == 0)
		m_currentMode = RegisterMode::P15;
	else
		m_currentMode = RegisterMode::MachineType;

	UpdateRegister();
}

void Input::UpdateRegister()
{
	if(m_currentMode == RegisterMode::P14)
	{
		m_joypadRegister = m_buttonStates & 0x0f;
	}
	else if(m_currentMode == RegisterMode::P15)
	{
		m_joypadRegister = (m_buttonStates >> 4) & 0x0f;
	}
	else //(m_currentMode == RegisterMode::MachineType)
	{
		m_joypadRegister = 0xff;
	}
}

void Input::UpdateInterruptFlag()
{
	//Nothing pressed, so clear the interrupt flag
	if(m_buttonStates == 0xff)
	{
		u8 interrupts = m_machine->GetGbMemory()->Read8(REG_IF);
		interrupts &= ~(IF_INPUT);
		m_machine->GetGbMemory()->Write8(REG_IF, interrupts);
	}

	//Something's pressed, so set the interrupt flag
	else
	{
		u8 interrupts = m_machine->GetGbMemory()->Read8(REG_IF);
		interrupts |= IF_INPUT;
		m_machine->GetGbMemory()->Write8(REG_IF, interrupts);
	}
}
