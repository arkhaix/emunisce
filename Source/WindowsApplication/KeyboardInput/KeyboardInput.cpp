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
#include <string>
using namespace std;

#include "PlatformIncludes.h"

#include "MachineIncludes.h"

#include "../Emunisce/Emunisce.h"	///<todo: this is basically unused.  refactor this.

namespace Emunisce
{

class KeyboardInput_Private
{
public:

	EmunisceApplication* _Phoenix;

	IEmulatedMachine* _Machine;
	IEmulatedInput* _Input;

	multimap<string, int> _NameKeyMap;
	map<int, unsigned int> _KeyMap;		///<Built from _NameKeyMap
	map<int, bool> _KeyStates;

	KeyboardInput_Private()
	{
		_Phoenix = NULL;
		_Machine = NULL;
		_Input = NULL;

		_NameKeyMap.insert( make_pair("Up", VK_UP) );
		_NameKeyMap.insert( make_pair("Down", VK_DOWN) );
		_NameKeyMap.insert( make_pair("Left", VK_LEFT) );
		_NameKeyMap.insert( make_pair("Right", VK_RIGHT) );

		_NameKeyMap.insert( make_pair("B", 'Q') );
		_NameKeyMap.insert( make_pair("B", 'A') );
		_NameKeyMap.insert( make_pair("B", 'Z') );

		_NameKeyMap.insert( make_pair("A", 'W') );
		_NameKeyMap.insert( make_pair("A", 'S') );
		_NameKeyMap.insert( make_pair("A", 'X') );

		_NameKeyMap.insert( make_pair("Select", 'V') );
		_NameKeyMap.insert( make_pair("Start", 'B') );

		_NameKeyMap.insert( make_pair("Select", VK_LSHIFT) );
		_NameKeyMap.insert( make_pair("Select", VK_RSHIFT) );
		_NameKeyMap.insert( make_pair("Start", VK_RETURN) );

		_NameKeyMap.insert( make_pair("Select", VK_OEM_4) );
		_NameKeyMap.insert( make_pair("Start", VK_OEM_6) );

		_NameKeyMap.insert( make_pair("Rewind", VK_TAB) );
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

	void GenerateKeymap()
	{
		if(_Input == NULL)
			return;

		_KeyMap.clear();
		_KeyStates.clear();

		for(unsigned int i=0;i<_Input->NumButtons();i++)
		{
			string buttonName = _Input->GetButtonName(i);
			auto mappedKeys = _NameKeyMap.equal_range(buttonName);
			for(auto iter = mappedKeys.first; iter != mappedKeys.second; iter++)
			{
				_KeyMap[iter->second] = i;
			}
		}
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
}

void KeyboardInput::Shutdown()
{
}

void KeyboardInput::SetMachine(IEmulatedMachine* machine)
{
	m_private->_Machine = machine;
	m_private->_Input = machine->GetInput();
	m_private->GenerateKeymap();
}

void KeyboardInput::KeyDown(int key)
{
	m_private->KeyDown(key);
}

void KeyboardInput::KeyUp(int key)
{
	m_private->KeyUp(key);
}
