#include "machine.h"

#include "types.h"

#include "../cpu/cpu.h"
#include "../display/display.h"
#include "../input/input.h"
#include "../memory/memory.h"

Machine* Machine::Create(const char* filename)
{
	Memory* memory = Memory::CreateFromFile(filename);
	if(memory == NULL)
		return NULL;

	Machine* result = new Machine();

	result->m_cpu = new Cpu();
	result->m_memory = memory;
	result->m_display = new Display();
	result->m_input = new Input();

	result->m_cpu->SetMachine(result);
	result->m_memory->SetMachine(result);
	result->m_display->SetMachine(result);
	result->m_input->SetMachine(result);

	result->m_cpu->Initialize();
	result->m_memory->Initialize();
	result->m_display->Initialize();
	result->m_input->Initialize();

	result->Initialize();

	return result;
}

void Machine::Release(Machine* machine)
{
	if(machine == NULL)
		return;

	delete machine->m_input;
	delete machine->m_display;
	delete machine->m_cpu;
	delete machine->m_memory;

	delete machine;
}

Machine::Machine()
{
	m_machineType = MachineType::GameBoy;
	m_frameCount = 0;

	m_cpu = NULL;
	m_memory = NULL;
	m_display = NULL;
	m_input = NULL;
}

void Machine::Initialize()
{
	m_frameCount = 0;

	m_ticksPerFrame = 69905;	///<4194304 / 60
	m_frameTicksRemaining = m_ticksPerFrame;
}

//Information
MachineType::Type Machine::GetMachineType()
{
	return m_machineType;
}

unsigned int Machine::GetFrameCount()
{
	return m_frameCount;
}


//Components
Cpu* Machine::GetCpu()
{
	return m_cpu;
}

Memory* Machine::GetMemory()
{
	return m_memory;
}

Display* Machine::GetDisplay()
{
	return m_display;
}

Input* Machine::GetInput()
{
	return m_input;
}


//Execution
void Machine::Step()
{
	int ticks = m_cpu->Step();
	if(m_cpu->IsStopped() == false)
		m_display->Run(ticks);

	m_frameTicksRemaining -= ticks;
	if(m_frameTicksRemaining<= 0)
	{
		m_frameCount++;
		m_frameTicksRemaining += m_ticksPerFrame;
	}
}

void Machine::RunOneFrame()
{
	unsigned int currentFrame = m_frameCount;
	while(m_frameCount == currentFrame)
		Step();
}

void Machine::Run()
{
	//todo
}

void Machine::Stop()
{
	//todo
}


//Persistence
bool Machine::SaveState(const char* filename)
{
	//todo
	return false;
}

bool Machine::LoadState(const char* filename)
{
	//todo
	return false;
}


//Debugging
void Machine::EnableBreakpoint(int address)
{
	//todo
	return;
}

void Machine::DisableBreakpoint(int address)
{
	//todo
	return;
}
