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
#ifndef CONSOLEDEBUGGER_SDL_H
#define CONSOLEDEBUGGER_SDL_H

#include <set>
#include <string>
#include <vector>

#include "machine_includes.h"
#include "user_interface.h"

namespace emunisce {

class EmunisceApplication;

class ConsoleDebugger {
public:
	ConsoleDebugger();

	void Initialize(EmunisceApplication* application);
	void Shutdown();

	void SetMachine(EmulatedMachine* machine);

	void Run();

	void Print(const char* text);

private:
	void Help();
	void UpdateDisplay();
	void FetchCommand();
	std::string FetchLine();
	std::vector<std::string> SplitCommand(std::string command);

	EmunisceApplication* m_application;
	IUserInterface* m_userInterface;

	EmulatedMachine* m_machine;
	EmulatedProcessor* m_cpu;
	EmulatedDisplay* m_display;
	EmulatedMemory* m_memory;

	bool m_muteSound;
	bool m_breakpointsEnabled;

	SquareSynthesisMethod::Type m_squareSynthesisMethod;
	DisplayFilter::Type m_displayFilter;

	bool m_recordingInput;
	bool m_playingInput;

	std::set<int> m_breakpoints;
};

}  // namespace emunisce

#endif
