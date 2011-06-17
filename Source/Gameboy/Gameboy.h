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
#ifndef GAMEBOY_H
#define GAMEBOY_H

#include "PlatformTypes.h"

#include "MachineIncludes.h"
#include "GameboyTypes.h"


namespace Emunisce
{

class Gameboy : public IEmulatedMachine
{
public:

	// IEmulatedMachine

	//Machine type
	virtual EmulatedMachine::Type GetType();

	//Component access
	virtual IEmulatedDisplay* GetDisplay();
	virtual IEmulatedInput* GetInput();
	virtual IEmulatedMemory* GetMemory();
	virtual IEmulatedProcessor* GetProcessor();
	virtual IEmulatedSound* GetSound();

	//Machine info
	virtual unsigned int GetFrameCount();
	virtual unsigned int GetTicksPerSecond();
	virtual unsigned int GetTicksUntilNextFrame();

	//Execution
	virtual void Step();
	virtual void RunOneFrame();
	virtual void Run();
	virtual void Stop();

	//Persistence
	virtual bool SaveState(const char* filename);
	virtual bool LoadState(const char* filename);

	//Debugging
	virtual void EnableBreakpoint(int address);
	virtual void DisableBreakpoint(int address);


	// Gameboy

	//Creation
	static Gameboy* Create(const char* filename);
	static void Release(Gameboy* machine);

	//Gameboy Components (non-virtual functions, direct concrete types)
	Cpu* GetGbCpu();
	Memory* GetGbMemory();
	Display* GetGbDisplay();
	Input* GetGbInput();
	Sound* GetGbSound();

	//Execution
	void RunDuringInstruction(unsigned int ticks);	///<Should only be called by the CPU.  Won't do anything if called externally.

protected:

	Gameboy(Memory* memory);
	~Gameboy();
	void Initialize();

	void InternalStep();	///<Non-virtual Step.

	Cpu* m_cpu;
	Memory* m_memory;
	Display* m_display;
	Input* m_input;
	Sound* m_sound;

	unsigned int m_frameCount;

	unsigned int m_ticksPerSecond;
	unsigned int m_ticksPerFrame;
	int m_frameTicksRemaining;

	bool m_executingInstruction;
	unsigned int m_subInstructionTicksSpent;	///<Tracks how many ticks were used during instruction execution
};

}	//namespace Emunisce

#endif