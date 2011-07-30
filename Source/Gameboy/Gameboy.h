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

#include "PlatformIncludes.h"

#include "MachineIncludes.h"
#include "GameboyTypes.h"

#include <algorithm>
#include <list>
using namespace std;


namespace Emunisce
{

class Gameboy : public IEmulatedMachine
{
public:

	// IEmulatedMachine

	//Machine type
	virtual EmulatedMachine::Type GetType();
	virtual const char* GetRomTitle();

	//Application interface
	virtual void SetApplicationInterface(IMachineToApplication* applicationInterface);
	virtual void AddApplicationEvent(ApplicationEvent& applicationEvent, bool relativeFrameCount);
	virtual void RemoveApplicationEvent(unsigned int eventId);

	//Component access
	virtual IEmulatedDisplay* GetDisplay();
	virtual IEmulatedInput* GetInput();
	virtual IEmulatedMemory* GetMemory();
	virtual IEmulatedProcessor* GetProcessor();
	virtual IEmulatedSound* GetSound();

	//Machine info
	virtual unsigned int GetFrameCount();
	virtual unsigned int GetTickCount();
	virtual unsigned int GetTicksPerSecond();
	virtual unsigned int GetTicksUntilNextFrame();

	//Execution
	virtual void Step();
	virtual void RunToNextFrame();
	virtual void Run();
	virtual void Stop();

	//Persistence
	virtual void SaveState(Archive& archive);
	virtual void LoadState(Archive& archive);

	//Debugging
	virtual void EnableBreakpoint(int address);
	virtual void DisableBreakpoint(int address);


	// Gameboy

	//Creation
	static Gameboy* Create(const char* filename, EmulatedMachine::Type machineType = EmulatedMachine::Gameboy);
	static void Release(Gameboy* machine);

	//Application interface
	IMachineToApplication* GetApplicationInterface();

	//Gameboy Components (non-virtual functions, direct concrete types)
	Cpu* GetGbCpu();
	Memory* GetGbMemory();
	Display* GetGbDisplay();
	Input* GetGbInput();
	Sound* GetGbSound();

	//Execution
	void RunDuringInstruction(unsigned int ticks);	///<Should only be called by the CPU.  Won't do anything if called externally.

protected:

	Gameboy(Memory* memory, EmulatedMachine::Type machineType);
	~Gameboy();
	void Initialize();

	void InternalStep();	///<Non-virtual Step.

	virtual void Serialize(Archive& archive);

	EmulatedMachine::Type m_machineType;

	IMachineToApplication* m_applicationInterface;

	Cpu* m_cpu;
	Memory* m_memory;
	Display* m_display;
	Input* m_input;
	Sound* m_sound;

	char m_romTitle[16];

	unsigned int m_frameCount;

	unsigned int m_ticksPerSecond;
	unsigned int m_ticksPerFrame;
	int m_frameTicksRemaining;

	bool m_executingInstruction;
	unsigned int m_subInstructionTicksSpent;	///<Tracks how many ticks were used during instruction execution

	list<ApplicationEvent> m_applicationEvents;
	list<ApplicationEvent>::iterator m_nextApplicationEvent;
	Mutex m_applicationEventsLock;
};

}	//namespace Emunisce

#endif
