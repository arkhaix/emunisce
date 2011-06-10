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
#ifndef MACHINE_H
#define MACHINE_H

class Cpu;
class Memory;
class Display;
class Input;
class Sound;

namespace MachineType
{
	typedef int Type;

	enum
	{
		GameBoy,
		GameBoyColor,

		NumMachineTypes
	};

	static const char* ToString[] =
	{
		"GameBoy",
		"GameBoyColor",

		"NumMachineTypes"
	};
}

class Machine
{
public:

	//Creation
	static Machine* Create(const char* filename);
	static void Release(Machine* machine);

	//Information
	MachineType::Type GetMachineType();
	unsigned int GetFrameCount();
	unsigned int GetTicksPerSecond();

	//Components
	Cpu* GetCpu();
	Memory* GetMemory();
	Display* GetDisplay();
	Input* GetInput();
	Sound* GetSound();

	//Execution
	void Step();
	void RunOneFrame();
	void Run();
	void Stop();
	void RunDuringInstruction(unsigned int ticks);	///<Should only be called by the CPU.  Won't do anything if called externally.

	//Persistence
	bool SaveState(const char* filename);
	bool LoadState(const char* filename);

	//Debugging
	void EnableBreakpoint(int address);
	void DisableBreakpoint(int address);

protected:

	Machine();
	void Initialize();

	MachineType::Type m_machineType;

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

#endif
