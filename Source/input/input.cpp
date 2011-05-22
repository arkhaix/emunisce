#include "input.h"

#include "../memory/memory.h"

Input::Input()
{
}

//Component
void Input::SetMachine(Machine* machine)
{
	if(machine && machine->_Memory)
	{
		machine->_Memory->SetRegisterLocation(0x00, &m_joypadRegister, false);
	}
}

void Input::Initialize()
{
	Reset();
}

void Input::Reset()
{
	m_buttonStates = 0xff;

	m_currentMode = RegisterMode::MachineType;
	m_joypadRegister = 0xff;
}

//External
void Input::ButtonDown(Buttons::Type button)
{
	m_buttonStates |= (1<<button);

	if(m_currentMode == RegisterMode::P14 || m_currentMode == RegisterMode::P15)
		UpdateRegister();
}

void Input::ButtonUp(Buttons::Type button)
{
	m_buttonStates &= ~(1<<button);

	if(m_currentMode == RegisterMode::P14 || m_currentMode == RegisterMode::P15)
		UpdateRegister();
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