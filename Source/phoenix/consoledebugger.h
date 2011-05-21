#ifndef CONSOLEDEBUGGER_H
#define CONSOLEDEBUGGER_H

#include "windows.h"

#include <set>
#include <string>
#include <vector>
using namespace std;

#include "../cpu/cpu.h"
#include "../display/display.h"
#include "../memory/memory.h"

class ConsoleDebugger
{
public:

	ConsoleDebugger();

	void Run(Machine* machine);

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

	void ToggleBreakpoint(int address);
	void ListBreakpoints();
	void ClearBreakpoints();

	void PrintMemory(int address, int length);


	//Properties

	Machine* m_machine;

	string m_lastFileLoaded;
	int m_frameTicksRemaining;

	bool m_requestingExit;

	set<u16> m_breakpoints;
};

#endif
