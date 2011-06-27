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
#include "KeyboardInput.h"
using namespace Emunisce;

#include "windows.h"

#include <map>
using namespace std;

#include "PlatformIncludes.h"

#include "MachineIncludes.h"

#include "../Emunisce/Emunisce.h"	///<todo: this is basically unused.  refactor this.
#include "../Emunisce/MachineRunner.h"

namespace Emunisce
{

class KeyboardInput_Private : public IWindowMessageListener
{
public:

	EmunisceApplication* _Phoenix;

	IEmulatedMachine* _Machine;
	IEmulatedInput* _Input;

	map<int, unsigned int> _KeyMap;
	map<int, bool> _KeyStates;

	KeyboardInput_Private()
	{
		_Phoenix = NULL;
		_Machine = NULL;
		_Input = NULL;

		/*
		GameboyButtons
		{
		Right = 0,
		Left,
		Up,
		Down,

		A,
		B,
		Select,
		Start,
		}
		*/
		_KeyMap[VK_UP] = 2;
		_KeyMap[VK_DOWN] = 3;
		_KeyMap[VK_LEFT] = 1;
		_KeyMap[VK_RIGHT] = 0;

		_KeyMap['Q'] = 5;
		_KeyMap['A'] = 5;
		_KeyMap['Z'] = 5;

		_KeyMap['W'] = 4;
		_KeyMap['S'] = 4;
		_KeyMap['X'] = 4;

		_KeyMap['V'] = 6;
		_KeyMap['B'] = 7;

		_KeyMap[VK_LSHIFT] = 6;
		_KeyMap[VK_RSHIFT] = 6;
		_KeyMap[VK_RETURN] = 7;

		_KeyMap[VK_OEM_4] = 6;
		_KeyMap[VK_OEM_6] = 7;
	}


	void Closed()
	{
	}

	void Draw()
	{
	}


	void Resize()
	{
	}


	void KeyDown(int key)
	{
		auto keyIter = _KeyMap.find(key);
		if(keyIter == _KeyMap.end())
			return;

		auto keyStateIter = _KeyStates.find(key);
		if(keyStateIter != _KeyStates.end() && keyStateIter->second == true)
			return;

		_KeyStates[key] = true;

		_Input->ButtonDown(keyIter->second);
	}

	void KeyUp(int key)
	{
		auto keyIter = _KeyMap.find(key);
		if(keyIter == _KeyMap.end())
			return;

		auto keyStateIter = _KeyStates.find(key);
		if(keyStateIter != _KeyStates.end() && keyStateIter->second == false)
			return;

		_KeyStates[key] = false;

		_Input->ButtonUp(keyIter->second);
	}
};

}	//namespace Emunisce

KeyboardInput::KeyboardInput()
{
	m_private = new KeyboardInput_Private();
}

void KeyboardInput::Initialize(EmunisceApplication* phoenix)
{
	m_private->_Phoenix = phoenix;
	
	phoenix->GetWindow()->SubscribeListener(m_private);
}

void KeyboardInput::Shutdown()
{
	m_private->_Phoenix->GetWindow()->UnsubscribeListener(m_private);
}

void KeyboardInput::SetMachine(IEmulatedMachine* machine)
{
	m_private->_Machine = machine;
	m_private->_Input = machine->GetInput();
}
