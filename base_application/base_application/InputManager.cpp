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
#include "InputManager.h"
using namespace emunisce;

#include "BaseApplication.h"
#include "MachineIncludes.h"
#include "PlatformIncludes.h"

InputManager::InputManager() {
	m_machine = nullptr;
	m_input = nullptr;
}

void InputManager::Initialize(BaseApplication* application) {
	m_application = application;
}

void InputManager::SetMachine(IEmulatedMachine* machine) {
	m_machine = machine;
	m_input = machine->GetInput();
	GenerateKeymap();
}

void InputManager::KeyDown(int key) {
	auto keyIter = m_keyMap.find(key);
	if (keyIter == m_keyMap.end()) {
		return;
	}

	auto keyStateIter = m_keyStates.find(key);
	if (keyStateIter != m_keyStates.end() && keyStateIter->second == true) {
		return;
	}

	m_keyStates[key] = true;

	m_input->ButtonDown(keyIter->second);
}

void InputManager::KeyUp(int key) {
	auto keyIter = m_keyMap.find(key);
	if (keyIter == m_keyMap.end()) {
		return;
	}

	auto keyStateIter = m_keyStates.find(key);
	if (keyStateIter != m_keyStates.end() && keyStateIter->second == false) {
		return;
	}

	m_keyStates[key] = false;

	m_input->ButtonUp(keyIter->second);
}

void InputManager::MapKey(const char* name, int keyCode) {
	m_nameKeyMap.insert(std::make_pair(name, keyCode));
	GenerateKeymap();
}

void InputManager::GenerateKeymap() {
	if (m_input == nullptr) {
		return;
	}

	m_keyMap.clear();
	m_keyStates.clear();

	for (unsigned int i = 0; i < m_input->NumButtons(); i++) {
		std::string buttonName = m_input->GetButtonName(i);
		auto mappedKeys = m_nameKeyMap.equal_range(buttonName);
		for (auto iter = mappedKeys.first; iter != mappedKeys.second; iter++) {
			m_keyMap[iter->second] = i;
		}
	}
}
