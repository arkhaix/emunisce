/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "ConsoleDebugger.h"

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
#include "../WindowsPlatform/Window.h"

//Machine
#include "../Machine/Machine.h"

//Application
#include "Phoenix.h"
#include "../WaveOutSound/WaveOutSound.h"


ConsoleDebugger::ConsoleDebugger()
{
	m_machine = NULL;
	m_cpu = NULL;
	m_display = NULL;
	m_memory = NULL;

	m_muteSound = false;

	m_breakpointsEnabled = false;
}

void ConsoleDebugger::Initialize(Phoenix* phoenix)
{
	m_phoenix = phoenix;
	phoenix->GetSound()->SetMute(m_muteSound);
}

void ConsoleDebugger::Shutdown()
{
}

void ConsoleDebugger::SetMachine(Machine* machine)
{
	m_machine = machine;

	m_cpu = machine->GetCpu();
	m_display = machine->GetDisplay();
	m_memory = machine->GetMemory();
}

void ConsoleDebugger::Run()
{
	SetupConsole();

	while(m_machine == NULL)
	{
		printf("\nFile: ");
		string filename = "";
		getline(cin, filename);

		LoadROM(filename.c_str());
	}

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
	system("cls");

	COORD position;
	position.X = position.Y = 0;
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), position);

	printf("AF: %04X\n", m_cpu->af);
	printf("BC: %04X\n", m_cpu->bc);
	printf("DE: %04X\n", m_cpu->de);
	printf("HL: %04X\n", m_cpu->hl);
	printf("\n");
	printf("PC: %04X\n", m_cpu->pc);
	printf("SP: %04X\n", m_cpu->sp);
	printf("\n");

	printf("Current opcode: ");
	u16 address = m_cpu->pc;
	for(int i=0;i<5;i++)
		printf("%02X ", m_memory->Read8(address++));
	printf("\n");

	printf("\n");
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

	COMMAND0("memory", PrintMemory(m_cpu->pc, 16))
	COMMAND0("mem", PrintMemory(m_cpu->pc, 16))
	COMMAND0("m", PrintMemory(m_cpu->pc, 16))

	COMMAND0("audio", ToggleMute())
	COMMAND0("mute", ToggleMute())
	COMMAND0("sound", ToggleMute())

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
	char* context = NULL;

	token = strtok_s(input, separators, &context);
	while(token != NULL)
	{
		result.push_back(string(token));
		token = strtok_s(NULL, separators, &context);
	}

	return result;
}


//Commands

void ConsoleDebugger::LoadROM(const char* filename)
{
	printf("%s(%s)\n", __FUNCTION__, filename);

	if(filename == NULL || strlen(filename) == 0)
		return;

	string sFilename = string("C:/hg/Phoenix/Roms/") + string(filename);
	if(sFilename.find(".gb") == string::npos && sFilename.find(".GB") == string::npos)
		sFilename += string(".gb");

	Machine* machine = Machine::Create(sFilename.c_str());
	if(machine == NULL)
	{
		printf("Failed to load the ROM\n");
		system("pause");
		return;
	}

	//Successfully created a new Machine
	Machine* oldMachine = m_machine;

	//Let everyone (including this class) know that the old one is going away
	m_phoenix->NotifyMachineChanged(machine);

	m_lastFileLoaded = filename;

	//Release the old one
	if(oldMachine != NULL)
		Machine::Release(oldMachine);

	printf("Success\n");
	Sleep(500);

	RunMachine();
}

void ConsoleDebugger::Reset()
{
	printf("%s\n", __FUNCTION__);

	LoadROM(m_lastFileLoaded.c_str());
}


void ConsoleDebugger::StepInto()
{
	//printf("%s\n", __FUNCTION__);

	m_machine->Step();
}

void ConsoleDebugger::StepOver()
{
	printf("%s\n", __FUNCTION__);
	printf("Not implemented\n");
}

void ConsoleDebugger::RunMachine()
{
	printf("%s\n", __FUNCTION__);

	printf("Press any key to pause execution...\n");

	if(m_breakpointsEnabled == false)
	{
		SetForegroundWindow((HWND)m_phoenix->GetWindow()->GetHandle());
		SetFocus((HWND)m_phoenix->GetWindow()->GetHandle());
	}

	int syncsPerSecond = 20;

	LARGE_INTEGER performanceFrequency;
	LARGE_INTEGER countsPerSync;
	QueryPerformanceFrequency(&performanceFrequency);
	countsPerSync.QuadPart = performanceFrequency.QuadPart / (LONGLONG)syncsPerSecond;

	double countsPerMillisecond = performanceFrequency.QuadPart / 1000.0;
	double millisecondsPerCount = 1.0 / countsPerMillisecond;

	int ticksPerSecond = 4194304;
	int ticksPerSync = ticksPerSecond / syncsPerSecond;

	int ticksUntilSync = ticksPerSync;

	LARGE_INTEGER syncPeriodStartCount;
	QueryPerformanceCounter(&syncPeriodStartCount);


	bool keepGoing = true;
	while(keepGoing)
	{
		LARGE_INTEGER curCount;

		int curFrame = m_machine->GetFrameCount();

		while(m_machine->GetFrameCount() == curFrame)
		{
			StepInto();
			if(m_breakpointsEnabled == true && m_breakpoints.find( m_cpu->pc ) != m_breakpoints.end())
			{
				keepGoing = false;
				break;
			}
		}

		ticksUntilSync -= 69905;	///<ish
		if(ticksUntilSync <= 0)
		{
			do
			{
				QueryPerformanceCounter(&curCount);
				LONGLONG countsTooFast = countsPerSync.QuadPart - (curCount.QuadPart - syncPeriodStartCount.QuadPart);
				if(countsTooFast <= 0)
					break;
				double millisecondsTooFast = millisecondsPerCount * countsTooFast;
				Sleep((DWORD)millisecondsTooFast);
			} while(curCount.QuadPart - syncPeriodStartCount.QuadPart < countsPerSync.QuadPart);

			syncPeriodStartCount.QuadPart = curCount.QuadPart;
			ticksUntilSync += ticksPerSync;
		}

		if(_kbhit())
			keepGoing = false;
	}
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
	system("cls");

	printf("%s(%x, %d)\n", __FUNCTION__, address, length);

	printf("\n");
	for(int i=0;i<length;i++)
	{
		if( i%16 == 0 )
			printf("\n%04X:\t", (u16)address);
		else if( i%8 == 0 )
			printf("\t");

		printf("%02X ", m_memory->Read8((u16)address));
		address++;
	}
	printf("\n");

	system("pause");
}

void ConsoleDebugger::ToggleMute()
{
	printf("%s\n", __FUNCTION__);

	m_muteSound = !m_muteSound;
	m_phoenix->GetSound()->SetMute(m_muteSound);
}
