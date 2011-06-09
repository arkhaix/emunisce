#include "Machine.h"

#include "Types.h"

#include "../Cpu/Cpu.h"
#include "../Display/Display.h"
#include "../Input/Input.h"
#include "../Memory/Memory.h"
#include "../Sound/Sound.h"

Machine* Machine::Create(const char* filename)
{
	Memory* memory = Memory::CreateFromFile(filename);
	if(memory == NULL)
		return NULL;

	Machine* result = new Machine();

	result->m_memory = memory;
	result->m_cpu = new Cpu();
	result->m_display = new Display();
	result->m_input = new Input();
	result->m_sound = new Sound();

	result->Initialize();

	result->m_memory->SetMachine(result);
	result->m_cpu->SetMachine(result);
	result->m_display->SetMachine(result);
	result->m_input->SetMachine(result);
	result->m_sound->SetMachine(result);

	result->m_memory->Initialize();
	result->m_cpu->Initialize();
	result->m_display->Initialize();
	result->m_input->Initialize();
	result->m_sound->Initialize();

	return result;
}

void Machine::Release(Machine* machine)
{
	if(machine == NULL)
		return;

	delete machine->m_sound;
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

	m_memory = NULL;
	m_cpu = NULL;
	m_display = NULL;
	m_input = NULL;
	m_sound = NULL;

	m_executingInstruction = false;
	m_subInstructionTicksSpent = 0;
}

void Machine::Initialize()
{
	m_frameCount = 0;

	m_ticksPerSecond = 4194304;
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

unsigned int Machine::GetTicksPerSecond()
{
	return m_ticksPerSecond;
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

Sound* Machine::GetSound()
{
	return m_sound;
}

//Execution
void Machine::Step()
{
	m_executingInstruction = true;
	unsigned int ticks = m_cpu->Step();
	m_executingInstruction = false;

	unsigned int totalTicks = ticks;

	//If the instruction spent more ticks than its total time (should never happen)
	// then roll over the spent ticks to the next instruction
	if(ticks < m_subInstructionTicksSpent)
	{
		m_subInstructionTicksSpent -= ticks;
		ticks = 0;
	}

	//Otherwise (normal case), just subtract the spent ticks from the total instruction time
	else
	{
		ticks -= m_subInstructionTicksSpent;
		m_subInstructionTicksSpent = 0;
	}

	m_cpu->RunTimer(ticks);

	if(m_cpu->IsStopped() == false)
		m_display->Run(ticks);

	m_sound->Run(ticks);

	m_frameTicksRemaining -= totalTicks;
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

void Machine::RunDuringInstruction(unsigned int ticks)
{
	if(m_executingInstruction == false)
		return;

	if(m_cpu->IsStopped() == false)
		m_display->Run(ticks);

	m_cpu->RunTimer(ticks);

	m_sound->Run(ticks);

	m_subInstructionTicksSpent += ticks;
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
