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
#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H


#include <map>
#include <string>


namespace Emunisce
{

	class BaseApplication;

	class IEmulatedInput;
	class IEmulatedMachine;

	class InputManager
	{
	public:

		InputManager();

		void Initialize(BaseApplication* application);

		void SetMachine(IEmulatedMachine* machine);

		void KeyDown(int key);
		void KeyUp(int key);

		void MapKey(const char* name, int keyCode); ///<name is the string provided by IEmulatedInput::GetButtonName.  keyCode is the value that will be passed into KeyDown/KeyUp.  todo: separate providers with identical keycodes.
		//todo: UnmapKey


	private:

		void GenerateKeymap();

		BaseApplication* m_application;

		IEmulatedMachine* m_machine;
		IEmulatedInput* m_input;

		std::multimap<std::string, int> m_nameKeyMap;
		std::map<int, unsigned int> m_keyMap;		///<Built from _NameKeyMap
		std::map<int, bool> m_keyStates;
	};

}	//namespace Emunisce




#endif
