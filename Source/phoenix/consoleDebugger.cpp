#include "consoledebugger.h"

#include "windows.h"

#include <conio.h>
#include <fcntl.h>
#include <io.h>

#include <iostream>
using namespace std;

ConsoleDebugger::ConsoleDebugger()
{
	m_requestingExit = false;
}

void ConsoleDebugger::Run(Machine* machine)
{
	SetupConsole();

	if(machine)
		m_machine = machine;
	else
		m_machine = new Machine();

	m_machine->_Memory = NULL;
	while(m_machine->_Memory == NULL)
	{
		printf("\nFile: ");
		string filename = "";
		getline(cin, filename);

		LoadROM(filename.c_str());
	}

	m_requestingExit = false;
	while(m_requestingExit == false)
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

	printf("AF: %04X\n", m_machine->_CPU->af);
	printf("BC: %04X\n", m_machine->_CPU->bc);
	printf("DE: %04X\n", m_machine->_CPU->de);
	printf("HL: %04X\n", m_machine->_CPU->hl);
	printf("\n");
	printf("PC: %04X\n", m_machine->_CPU->pc);
	printf("SP: %04X\n", m_machine->_CPU->sp);
	printf("\n");

	printf("Current opcode: ");
	u16 address = m_machine->_CPU->pc;
	for(int i=0;i<5;i++)
		printf("%02X ", m_machine->_Memory->Read8(address++));
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

	COMMAND0("quit", m_requestingExit = true)
	COMMAND0("q", m_requestingExit = true)
	COMMAND0("exit", m_requestingExit = true)
	COMMAND0("x", m_requestingExit = true)

	COMMAND1("load", LoadROM(args[1].c_str()))
	COMMAND1("l", LoadROM(args[1].c_str()))
	COMMAND1("open", LoadROM(args[1].c_str()))
	COMMAND1("o", LoadROM(args[1].c_str()))

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

	COMMAND0("memory", PrintMemory(m_machine->_CPU->pc, 16))
	COMMAND0("mem", PrintMemory(m_machine->_CPU->pc, 16))
	COMMAND0("m", PrintMemory(m_machine->_CPU->pc, 16))

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

	Memory* memory = Memory::CreateFromFile(sFilename.c_str());
	if(memory == NULL)
	{
		printf("Failed to load the ROM\n");
		system("pause");
		return;
	}

	m_machine->_Memory = memory;
	m_machine->_CPU = new CPU();
	m_machine->_Display = new Display();
	m_machine->_Input = new Input();

	m_machine->_Memory->SetMachine(m_machine);
	m_machine->_CPU->SetMachine(m_machine);
	m_machine->_Display->SetMachine(m_machine);
	m_machine->_Input->SetMachine(m_machine);

	m_machine->_Memory->Initialize();
	m_machine->_CPU->Initialize();
	m_machine->_Display->Initialize();
	m_machine->_Input->Initialize();

	m_machine->_FrameCount = 0;
	m_frameTicksRemaining = 69905;

	m_lastFileLoaded = filename;

	printf("Success\n");
	Sleep(500);
}

void ConsoleDebugger::Reset()
{
	printf("%s\n", __FUNCTION__);

	if(m_machine->_Memory)
		delete m_machine->_Memory;

	if(m_machine->_CPU)
		delete m_machine->_CPU;

	if(m_machine->_Display)
		delete m_machine->_Display;

	LoadROM(m_lastFileLoaded.c_str());
}


void ConsoleDebugger::StepInto()
{
	//printf("%s\n", __FUNCTION__);

	int ticks = m_machine->_CPU->Step();
	m_machine->_Display->Run(ticks);

	m_frameTicksRemaining -= ticks;
	if(m_frameTicksRemaining<= 0)
	{
		m_machine->_FrameCount++;
		m_frameTicksRemaining += 69905;
	}
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

	int syncsPerSecond = 30;

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

		int curFrame = m_machine->_FrameCount;

		while(m_machine->_FrameCount == curFrame)
		{
			StepInto();
			if(m_breakpoints.find( m_machine->_CPU->pc ) != m_breakpoints.end())
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

	RunMachine();

	if(addedBreakpoint && iter != m_breakpoints.end())
		m_breakpoints.erase(iter);
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

		printf("%02X ", m_machine->_Memory->Read8((u16)address));
		address++;
	}
	printf("\n");

	system("pause");
}

