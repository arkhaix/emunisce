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
#include "KeyboardInput.h"

#include "windows.h"

#include <map>
using namespace std;

#include "../Machine/Types.h"
#include "../Machine/Machine.h"
#include "../Input/Input.h"

#include "Phoenix.h"

class KeyboardInput_Private
{
public:

	Phoenix* _Phoenix;

	Machine* _Machine;
	Input* _Input;

	map<int, Buttons::Type> _KeyMap;

	KeyboardInput_Private()
	{
		_Phoenix = NULL;
		_Machine = NULL;
		_Input = NULL;
	}
};

KeyboardInput::KeyboardInput()
{
	m_private = new KeyboardInput_Private();
}

void KeyboardInput::Initialize(Phoenix* phoenix)
{
	m_private->_Phoenix = phoenix;

	m_private->_KeyMap[VK_UP] = Buttons::Up;
	m_private->_KeyMap[VK_DOWN] = Buttons::Down;
	m_private->_KeyMap[VK_LEFT] = Buttons::Left;
	m_private->_KeyMap[VK_RIGHT] = Buttons::Right;

	m_private->_KeyMap['Q'] = Buttons::B;
	m_private->_KeyMap['A'] = Buttons::B;
	m_private->_KeyMap['Z'] = Buttons::B;

	m_private->_KeyMap['W'] = Buttons::A;
	m_private->_KeyMap['S'] = Buttons::A;
	m_private->_KeyMap['X'] = Buttons::A;

	m_private->_KeyMap['V'] = Buttons::Select;
	m_private->_KeyMap['B'] = Buttons::Start;

	m_private->_KeyMap[VK_LSHIFT] = Buttons::Select;
	m_private->_KeyMap[VK_RSHIFT] = Buttons::Select;
	m_private->_KeyMap[VK_RETURN] = Buttons::Start;

	m_private->_KeyMap[VK_OEM_4] = Buttons::Select;
	m_private->_KeyMap[VK_OEM_6] = Buttons::Start;
}

void KeyboardInput::Shutdown()
{
}

void KeyboardInput::SetMachine(Machine* machine)
{
	m_private->_Machine = machine;
	m_private->_Input = machine->GetInput();
}

void KeyboardInput::KeyDown(int key)
{
	if(m_private->_Input == NULL)
		return;

	auto keyIter = m_private->_KeyMap.find(key);
	if(keyIter == m_private->_KeyMap.end())
		return;

	m_private->_Input->ButtonDown(keyIter->second);
}

void KeyboardInput::KeyUp(int key)
{
	if(m_private->_Input == NULL)
		return;

	auto keyIter = m_private->_KeyMap.find(key);
	if(keyIter == m_private->_KeyMap.end())
		return;

	m_private->_Input->ButtonUp(keyIter->second);
}
