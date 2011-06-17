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
#ifndef CONSOLEDEBUGGER_H
#define CONSOLEDEBUGGER_H

#include "windows.h"

#include <set>
#include <string>
#include <vector>
using namespace std;

#include "MachineIncludes.h"


namespace Emunisce
{

class EmunisceApplication;
class UserInterface;


class ConsoleDebugger
{
public:

	ConsoleDebugger();

	void Initialize(EmunisceApplication* phoenix);
	void Shutdown();

	void SetMachine(IEmulatedMachine* machine);

	void Run();

private:

	void SetupConsole();

	void UpdateDisplay();
	void FetchCommand();

	vector<string> SplitCommand(string command);


	//Commands

	void LoadROM(const char* filename);
	void Reset();

	void StepInto();
	void StepOver();
	void RunMachine();
	void RunMachineTo(int address);

	void Pause();

	void Speed(float multiplier);

	void ToggleBreakpoint(int address);
	void ListBreakpoints();
	void ClearBreakpoints();

	void PrintMemory(int address, int length);

	void ToggleMute();
	void SetSquareSynthesisMethod(const char* strMethod);

	void SetDisplayFilter(const char* strFilter);


	//Properties

	EmunisceApplication* m_phoenix;
	UserInterface* m_userInterface;

	IEmulatedMachine* m_machine;
	IEmulatedProcessor* m_cpu;
	IEmulatedDisplay* m_display;
	IEmulatedMemory* m_memory;

	string m_lastFileLoaded;
	int m_frameTicksRemaining;

	bool m_breakpointsEnabled;
	set<u16> m_breakpoints;

	bool m_muteSound;

	SquareSynthesisMethod::Type m_squareSynthesisMethod;
	DisplayFilter::Type m_displayFilter;
};

}	//namespace Emunisce

#endif
