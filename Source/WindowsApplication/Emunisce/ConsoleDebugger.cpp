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
#include "ConsoleDebugger.h"
using namespace Emunisce;

//Windows
#include "windows.h"

//STL
#include <iostream>
using namespace std;

//CRT
#include <conio.h>
#include <fcntl.h>
#include <io.h>

//Platform
#include "PlatformIncludes.h"

//Machine
#include "MachineIncludes.h"

//Application
#include "Emunisce.h"
#include "UserInterface.h"
#include "../WaveOutSound/WaveOutSound.h"


ConsoleDebugger::ConsoleDebugger()
{
	m_machine = NULL;
	m_cpu = NULL;
	m_display = NULL;
	m_memory = NULL;

	m_muteSound = false;

	m_breakpointsEnabled = false;

	m_squareSynthesisMethod = SquareSynthesisMethod::LinearInterpolation;
	m_displayFilter = DisplayFilter::None;
}

void ConsoleDebugger::Initialize(EmunisceApplication* phoenix)
{
	m_phoenix = phoenix;
	m_userInterface = phoenix->GetUserInterface();

	phoenix->GetSound()->SetMute(m_muteSound);
}

void ConsoleDebugger::Shutdown()
{
}

void ConsoleDebugger::SetMachine(IEmulatedMachine* machine)
{
	m_machine = machine;

	m_cpu = NULL;//machine->GetCpu();
	m_display = machine->GetDisplay();
	m_memory = NULL;//machine->GetMemory();

	machine->GetSound()->SetSquareSynthesisMethod(m_squareSynthesisMethod);
	machine->GetDisplay()->SetFilter(m_displayFilter);
}

void ConsoleDebugger::Run()
{
	SetupConsole();

	RunMachine();

	while(m_phoenix->ShutdownRequested() == false)
	{
		UpdateDisplay();
		FetchCommand();
	}
}


void ConsoleDebugger::SetupConsole()
{
	AllocConsole();

	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);
	freopen("CONIN$", "r", stdin);
}

void ConsoleDebugger::UpdateDisplay()
{
	//system("cls");

	//COORD position;
	//position.X = position.Y = 0;
	//SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);

	//printf("AF: %04X\n", m_cpu->af);
	//printf("BC: %04X\n", m_cpu->bc);
	//printf("DE: %04X\n", m_cpu->de);
	//printf("HL: %04X\n", m_cpu->hl);
	//printf("\n");
	//printf("PC: %04X\n", m_cpu->pc);
	//printf("SP: %04X\n", m_cpu->sp);
	//printf("\n");

	//printf("Current opcode: ");
	//u16 address = m_cpu->pc;
	//for(int i=0;i<5;i++)
	//	printf("%02X ", m_memory->Read8(address++));
	//printf("\n");

	//printf("\n");
}

void ConsoleDebugger::FetchCommand()
{
	string line = "";
	printf("\n> ");
	getline(cin, line);

	vector<string> args = SplitCommand(line);
	if(args.size() == 0)
		return;

	const char* command = args[0].c_str();


#define COMMAND0(name, target)	else if( _stricmp(command, name) == 0 ) { target; }
#define COMMAND1(name, target)	else if( _stricmp(command, name) == 0 && args.size() > 1 ) { target; }
#define COMMAND2(name, target)	else if( _stricmp(command, name) == 0 && args.size() > 2 ) { target; }

	if(false) { }

	COMMAND0("quit", m_phoenix->RequestShutdown())
	COMMAND0("q", m_phoenix->RequestShutdown())
	COMMAND0("exit", m_phoenix->RequestShutdown())
	COMMAND0("x", m_phoenix->RequestShutdown())

	COMMAND1("load", LoadROM(line.substr(args[0].size()+1).c_str()))
	COMMAND1("l", LoadROM(line.substr(args[0].size()+1).c_str()))
	COMMAND1("open", LoadROM(line.substr(args[0].size()+1).c_str()))
	COMMAND1("o", LoadROM(line.substr(args[0].size()+1).c_str()))

	COMMAND0("load", LoadROM(NULL))
	COMMAND0("l", LoadROM(NULL))
	COMMAND0("open", LoadROM(NULL))
	COMMAND0("o", LoadROM(NULL))

	COMMAND0("reset", Reset())

	COMMAND0("step", StepInto())
	COMMAND0("s", StepInto())

	COMMAND0("stepover", StepOver())
	COMMAND0("so", StepOver())

	COMMAND1("run", RunMachineTo(strtol(args[1].c_str(), NULL, 16)))
	COMMAND1("rt", RunMachineTo(strtol(args[1].c_str(), NULL, 16)))
	COMMAND1("r", RunMachineTo(strtol(args[1].c_str(), NULL, 16)))
	COMMAND1("go", RunMachineTo(strtol(args[1].c_str(), NULL, 16)))
	COMMAND1("gt", RunMachineTo(strtol(args[1].c_str(), NULL, 16)))
	COMMAND1("g", RunMachineTo(strtol(args[1].c_str(), NULL, 16)))

	COMMAND0("run", RunMachine())
	COMMAND0("r", RunMachine())
	COMMAND0("go", RunMachine())
	COMMAND0("g", RunMachine())

	COMMAND0("pause", Pause())
	COMMAND0("p", Pause())

	COMMAND1("speed", Speed((float)strtod(args[1].c_str(), NULL)))
	COMMAND1("sp", Speed((float)strtod(args[1].c_str(), NULL)))

	COMMAND1("savestate", SaveState(args[1].c_str()))
	COMMAND1("ss", SaveState(args[1].c_str()))
	COMMAND1("loadstate", LoadState(args[1].c_str()))
	COMMAND1("ls", LoadState(args[1].c_str()))

	COMMAND1("breakpoint", ToggleBreakpoint(strtol(args[1].c_str(), NULL, 16)))
	COMMAND1("break", ToggleBreakpoint(strtol(args[1].c_str(), NULL, 16)))
	COMMAND1("b", ToggleBreakpoint(strtol(args[1].c_str(), NULL, 16)))
	
	COMMAND0("breakpoints", ListBreakpoints())
	COMMAND0("breakpoint", ListBreakpoints())
	COMMAND0("break", ListBreakpoints())
	COMMAND0("b", ListBreakpoints())

	COMMAND0("clearbreakpoints", ClearBreakpoints())
	COMMAND0("cb", ClearBreakpoints())

	COMMAND2("memory", PrintMemory(strtol(args[1].c_str(), NULL, 16), strtol(args[2].c_str(), NULL, 16)))
	COMMAND2("mem", PrintMemory(strtol(args[1].c_str(), NULL, 16), strtol(args[2].c_str(), NULL, 16)))
	COMMAND2("m", PrintMemory(strtol(args[1].c_str(), NULL, 16), strtol(args[2].c_str(), NULL, 16)))

	COMMAND1("memory", PrintMemory(strtol(args[1].c_str(), NULL, 16), 16))
	COMMAND1("mem", PrintMemory(strtol(args[1].c_str(), NULL, 16), 16))
	COMMAND1("m", PrintMemory(strtol(args[1].c_str(), NULL, 16), 16))

	//COMMAND0("memory", PrintMemory(m_cpu->pc, 16))
	//COMMAND0("mem", PrintMemory(m_cpu->pc, 16))
	//COMMAND0("m", PrintMemory(m_cpu->pc, 16))

	COMMAND0("audio", ToggleMute())
	COMMAND0("mute", ToggleMute())
	COMMAND0("sound", ToggleMute())

	COMMAND1("square", SetSquareSynthesisMethod(args[1].c_str()))

	COMMAND1("displayfilter", SetDisplayFilter(args[1].c_str()))
	COMMAND1("display", SetDisplayFilter(args[1].c_str()))
	COMMAND1("filter", SetDisplayFilter(args[1].c_str()))
	COMMAND1("df", SetDisplayFilter(args[1].c_str()))

	else
	{
		printf("Unrecognized command\n");
	}
}

vector<string> ConsoleDebugger::SplitCommand(string command)
{
	vector<string> result;

	char* input = const_cast<char*>(command.c_str());
	const char* separators = " \t\n";
	char* token = NULL;

	token = strtok(input, separators);
	while(token != NULL)
	{
		result.push_back(string(token));
		token = strtok(NULL, separators);
	}

	return result;
}


//Commands

void ConsoleDebugger::LoadROM(const char* filename)
{
	printf("%s(%s)\n", __FUNCTION__, filename);

	m_userInterface->LoadRom(filename);

	if(m_breakpointsEnabled == false)
	{
		SetForegroundWindow((HWND)m_phoenix->GetWindow()->GetHandle());
		SetFocus((HWND)m_phoenix->GetWindow()->GetHandle());
	}
}

void ConsoleDebugger::Reset()
{
	printf("%s\n", __FUNCTION__);

	m_userInterface->Reset();
}


void ConsoleDebugger::StepInto()
{
	printf("%s\n", __FUNCTION__);

	m_userInterface->StepInstruction();
}

void ConsoleDebugger::StepOver()
{
	printf("%s\n", __FUNCTION__);
	printf("Not implemented\n");
}

void ConsoleDebugger::RunMachine()
{
	printf("%s\n", __FUNCTION__);

	if(m_breakpointsEnabled == false)
	{
		SetForegroundWindow((HWND)m_phoenix->GetWindow()->GetHandle());
		SetFocus((HWND)m_phoenix->GetWindow()->GetHandle());
	}

	m_userInterface->Run();
}

void ConsoleDebugger::RunMachineTo(int address)
{
	u16 gbAddress = (u16)address;
	bool addedBreakpoint = false;

	auto iter = m_breakpoints.find(gbAddress);
	if(iter == m_breakpoints.end())
	{
		m_breakpoints.insert(gbAddress);
		iter = m_breakpoints.find(gbAddress);
		addedBreakpoint = true;
	}

	m_breakpointsEnabled = true;

	RunMachine();

	if(addedBreakpoint && iter != m_breakpoints.end())
	{
		m_breakpoints.erase(iter);
		if(m_breakpoints.size() == 0)
			m_breakpointsEnabled = false;
	}
}


void ConsoleDebugger::Pause()
{
	m_userInterface->Pause();
}


void ConsoleDebugger::Speed(float multiplier)
{
	m_userInterface->SetEmulationSpeed(multiplier);
}


void ConsoleDebugger::SaveState(const char* id)
{
	m_userInterface->SaveState(id);
}

void ConsoleDebugger::LoadState(const char* id)
{
	m_userInterface->LoadState(id);
}


void ConsoleDebugger::ToggleBreakpoint(int address)
{
	printf("%s(%x)\n", __FUNCTION__, address);
	
	u16 gbAddress = (u16)address;
	auto iter = m_breakpoints.find(gbAddress);
	if(iter == m_breakpoints.end())
	{
		m_breakpoints.insert(gbAddress);
		printf("Enabled a breakpoint at: %04X\n", gbAddress);
	}
	else
	{
		m_breakpoints.erase(iter);
		printf("Removed a breakpoint at: %04X\n", gbAddress);
	}

	if(m_breakpoints.size() > 0)
		m_breakpointsEnabled = true;
	else
		m_breakpointsEnabled = false;

	Sleep(1000);
}

void ConsoleDebugger::ListBreakpoints()
{
	printf("%s\n", __FUNCTION__);

	printf("\nThere are %d breakpoint addresses:\n", m_breakpoints.size());
	printf("\n");

	for(auto iter = m_breakpoints.begin(); iter != m_breakpoints.end(); ++iter)
		printf("%04X\n", *iter);

	printf("---\n\n");
	system("pause");
}

void ConsoleDebugger::ClearBreakpoints()
{
	printf("%s\n", __FUNCTION__);

	auto numBreakpoints = m_breakpoints.size();
	m_breakpoints.clear();
	m_breakpointsEnabled = false;

	printf("\nCleared %d breakpoints\n", numBreakpoints);
	Sleep(1000);
}


void ConsoleDebugger::PrintMemory(int address, int length)
{
	//system("cls");

	//printf("%s(%x, %d)\n", __FUNCTION__, address, length);

	//printf("\n");
	//for(int i=0;i<length;i++)
	//{
	//	if( i%16 == 0 )
	//		printf("\n%04X:\t", (u16)address);
	//	else if( i%8 == 0 )
	//		printf("\t");

	//	printf("%02X ", m_memory->Read8((u16)address));
	//	address++;
	//}
	//printf("\n");

	//system("pause");
}

void ConsoleDebugger::ToggleMute()
{
	printf("%s\n", __FUNCTION__);

	m_muteSound = !m_muteSound;
	m_phoenix->GetSound()->SetMute(m_muteSound);
}

void ConsoleDebugger::SetSquareSynthesisMethod(const char* strMethod)
{
	printf("%s\n", __FUNCTION__);

	if(strMethod == NULL || strlen(strMethod) == 0)
		strMethod = "linear";

	SquareSynthesisMethod::Type method = SquareSynthesisMethod::LinearInterpolation;

	if( _stricmp(strMethod, "linear") == 0 || _stricmp(strMethod, "interpolation") == 0 ||
		_stricmp(strMethod, "interp") == 0 || _stricmp(strMethod, "1") == 0 )
		method = SquareSynthesisMethod::LinearInterpolation;

	else if( _stricmp(strMethod, "naive") == 0 || _stricmp(strMethod, "simple") == 0 ||
			_stricmp(strMethod, "0") == 0 )
		method = SquareSynthesisMethod::Naive;

	m_machine->GetSound()->SetSquareSynthesisMethod(method);
	m_squareSynthesisMethod = method;
}

void ConsoleDebugger::SetDisplayFilter(const char* strFilter)
{
	printf("%s\n", __FUNCTION__);

	if(strFilter == NULL || strlen(strFilter) == 0)
		strFilter = "none";

	DisplayFilter::Type filter = DisplayFilter::None;

	if( _stricmp(strFilter, "none") == 0 || _stricmp(strFilter, "0") == 0 ||
		_stricmp(strFilter, "1") == 0 )
		filter = DisplayFilter::None;

	else if( _stricmp(strFilter, "hq2x") == 0 || _stricmp(strFilter, "2x") == 0 ||
		_stricmp(strFilter, "2") == 0 )
		filter = DisplayFilter::Hq2x;

	else if( _stricmp(strFilter, "hq3x") == 0 || _stricmp(strFilter, "3x") == 0 ||
		_stricmp(strFilter, "3") == 0 )
		filter = DisplayFilter::Hq3x;

	else if( _stricmp(strFilter, "hq4x") == 0 || _stricmp(strFilter, "4x") == 0 ||
		_stricmp(strFilter, "4") == 0 )
		filter = DisplayFilter::Hq4x;

	m_machine->GetDisplay()->SetFilter(filter);
	m_displayFilter = filter;
}
