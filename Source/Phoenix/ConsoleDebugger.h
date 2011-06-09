#ifndef CONSOLEDEBUGGER_H
#define CONSOLEDEBUGGER_H

#include "windows.h"

#include <set>
#include <string>
#include <vector>
using namespace std;

#include "../Cpu/Cpu.h"
#include "../Display/Display.h"
#include "../Memory/Memory.h"
#include "../Input/Input.h"

class Phoenix;
class Machine;

class ConsoleDebugger
{
public:

	ConsoleDebugger();

	void Initialize(Phoenix* phoenix);
	void Shutdown();

	void SetMachine(Machine* machine);

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

	void ToggleBreakpoint(int address);
	void ListBreakpoints();
	void ClearBreakpoints();

	void PrintMemory(int address, int length);

	void ToggleMute();


	//Properties

	Phoenix* m_phoenix;

	Machine* m_machine;
	Cpu* m_cpu;
	Display* m_display;
	Memory* m_memory;

	string m_lastFileLoaded;
	int m_frameTicksRemaining;

	bool m_breakpointsEnabled;
	set<u16> m_breakpoints;

	bool m_muteSound;
};

#endif
