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
#include "Gameboy.h"
using namespace Emunisce;

//Gameboy
#include "GameboyIncludes.h"


// IEmulatedMachine

//Machine type
EmulatedMachine::Type Gameboy::GetType()
{
	return EmulatedMachine::Gameboy;
}


//Component access
IEmulatedDisplay* Gameboy::GetDisplay()
{
	return m_display;
}

IEmulatedInput* Gameboy::GetInput()
{
	return m_input;
}

IEmulatedMemory* Gameboy::GetMemory()
{
	return m_memory;
}

IEmulatedProcessor* Gameboy::GetProcessor()
{
	return m_cpu;
}

IEmulatedSound* Gameboy::GetSound()
{
	return m_sound;
}


//Machine info
unsigned int Gameboy::GetFrameCount()
{
	return m_frameCount;
}

unsigned int Gameboy::GetTicksPerSecond()
{
	return m_ticksPerSecond;
}

unsigned int Gameboy::GetTicksUntilNextFrame()
{
	return m_frameTicksRemaining;
}


//Execution
void Gameboy::Step()
{
	InternalStep();
}

void Gameboy::RunOneFrame()
{
	unsigned int currentFrame = m_frameCount;
	while(m_frameCount == currentFrame)
		InternalStep();
}

void Gameboy::Run()
{
}

void Gameboy::Stop()
{
}


//Persistence
bool Gameboy::SaveState(const char* filename)
{
	return false;
}

bool Gameboy::LoadState(const char* filename)
{
	return false;
}


//Debugging
void Gameboy::EnableBreakpoint(int address)
{
}

void Gameboy::DisableBreakpoint(int address)
{
}



// Gameboy

//Creation
Gameboy* Gameboy::Create(const char* filename)
{
	Memory* memory = Memory::CreateFromFile(filename);
	if(memory == NULL)
		return NULL;

	Gameboy* result = new Gameboy(memory);

	result->Initialize();


	return result;
}

void Gameboy::Release(Gameboy* machine)
{
	if(machine != NULL)
		delete machine;
}


//Gameboy Components (non-functions, direct concrete types)
Cpu* Gameboy::GetGbCpu()
{
	return m_cpu;
}

Memory* Gameboy::GetGbMemory()
{
	return m_memory;
}

Display* Gameboy::GetGbDisplay()
{
	return m_display;
}

Input* Gameboy::GetGbInput()
{
	return m_input;
}

Sound* Gameboy::GetGbSound()
{
	return m_sound;
}


//Execution
void Gameboy::RunDuringInstruction(unsigned int ticks)
{
	if(m_executingInstruction == false)
		return;

	if(m_cpu->IsStopped() == false)
		m_display->Run(ticks);

	m_cpu->RunTimer(ticks);

	m_sound->Run(ticks);

	m_subInstructionTicksSpent += ticks;
}


//protected:

Gameboy::Gameboy(Memory* memory)
{
	m_memory = memory;
	m_cpu = new Cpu();
	m_display = new Display();
	m_input = new Input();
	m_sound = new Sound();

	m_frameCount = 0;

	m_ticksPerSecond = 4194304;
	m_ticksPerFrame = 69905;	///<todo: 70224
	m_frameTicksRemaining = m_ticksPerFrame;

	m_executingInstruction = false;
	m_subInstructionTicksSpent = 0;
}

Gameboy::~Gameboy()
{
	delete m_sound;
	delete m_input;
	delete m_display;
	delete m_cpu;
	delete m_memory;
}

void Gameboy::Initialize()
{
	m_memory->SetMachine(this);
	m_cpu->SetMachine(this);
	m_display->SetMachine(this);
	m_input->SetMachine(this);
	m_sound->SetMachine(this);

	m_memory->Initialize();
	m_cpu->Initialize();
	m_display->Initialize();
	m_input->Initialize();
	m_sound->Initialize();
}

void Gameboy::InternalStep()
{
	m_executingInstruction = true;
	unsigned int ticks = m_cpu->Step();
	m_executingInstruction = false;

	unsigned int ticksThisStep = ticks;

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

	m_frameTicksRemaining -= ticksThisStep;
	if(m_frameTicksRemaining<= 0)
	{
		m_frameCount++;
		m_frameTicksRemaining += m_ticksPerFrame;
	}
}