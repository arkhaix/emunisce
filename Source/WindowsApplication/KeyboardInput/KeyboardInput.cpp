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

namespace Emunisce
{

class KeyboardInput_Private : public IWindowMessageListener
{
public:

	EmunisceApplication* _Phoenix;

	IEmulatedMachine* _Machine;
	IEmulatedInput* _Input;

	map<int, Buttons::Type> _KeyMap;

	KeyboardInput_Private()
	{
		_Phoenix = NULL;
		_Machine = NULL;
		_Input = NULL;

		_KeyMap[VK_UP] = Buttons::Up;
		_KeyMap[VK_DOWN] = Buttons::Down;
		_KeyMap[VK_LEFT] = Buttons::Left;
		_KeyMap[VK_RIGHT] = Buttons::Right;

		_KeyMap['Q'] = Buttons::B;
		_KeyMap['A'] = Buttons::B;
		_KeyMap['Z'] = Buttons::B;

		_KeyMap['W'] = Buttons::A;
		_KeyMap['S'] = Buttons::A;
		_KeyMap['X'] = Buttons::A;

		_KeyMap['V'] = Buttons::Select;
		_KeyMap['B'] = Buttons::Start;

		_KeyMap[VK_LSHIFT] = Buttons::Select;
		_KeyMap[VK_RSHIFT] = Buttons::Select;
		_KeyMap[VK_RETURN] = Buttons::Start;

		_KeyMap[VK_OEM_4] = Buttons::Select;
		_KeyMap[VK_OEM_6] = Buttons::Start;
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

		_Input->ButtonDown(keyIter->second);
	}

	void KeyUp(int key)
	{
		auto keyIter = _KeyMap.find(key);
		if(keyIter == _KeyMap.end())
			return;

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