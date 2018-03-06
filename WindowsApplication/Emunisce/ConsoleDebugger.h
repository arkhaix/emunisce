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

#include "BaseApplication/IUserInterface.h"	///<For DisplayFilter


namespace Emunisce
{

class EmunisceApplication;


class ConsoleDebugger
{
public:

	ConsoleDebugger();

	void Initialize(EmunisceApplication* phoenix);
	void Shutdown();

	void SetMachine(IEmulatedMachine* machine);

	void Run();

	void Print(const char* text);

private:

	void Help();

	void SetupConsole();

	void UpdateDisplay();
	void FetchCommand();
	string FetchLine();

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

	void SaveState(const char* id);
	void LoadState(const char* id);

	void ToggleBreakpoint(int address);
	void ListBreakpoints();
	void ClearBreakpoints();

	void PrintMemory(int address, int length);

	void ToggleMute();
	void SetSquareSynthesisMethod(const char* strMethod);

	void SetBackgroundAnimation(const char* state);

	void SetDisplayFilter(const char* strFilter);
	void SetVsync(const char* strMode);

	void ToggleRecording();

	void TogglePlayMovie();
	void TogglePlayMacro(const char* loop);

	void SaveMovie(const char* id);
	void LoadMovie(const char* id);

	void SaveMacro(const char* id);
	void LoadMacro(const char* id);

	void PrintButtons();

	void PrintMachineType();


	//Properties

	EmunisceApplication* m_phoenix;
	IUserInterface* m_userInterface;

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

	bool m_recordingInput;
	bool m_playingInput;
};

}	//namespace Emunisce

#endif
