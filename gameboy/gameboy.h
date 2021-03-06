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

#include <algorithm>
#include <list>
#include <mutex>

#include "gameboy_types.h"
#include "machine_includes.h"
#include "platform_includes.h"

namespace emunisce {

class Gameboy : public EmulatedMachine {
public:
	// EmulatedMachine

	// Machine type
	Machine::Type GetType() override;
	const char* GetRomTitle() override;

	// Application interface
	void SetApplicationInterface(MachineToApplication* applicationInterface) override;
	void AddApplicationEvent(ApplicationEvent& applicationEvent, bool relativeFrameCount) override;
	void RemoveApplicationEvent(unsigned int eventId) override;

	// Component access
	EmulatedDisplay* GetDisplay() override;
	EmulatedInput* GetInput() override;
	EmulatedMemory* GetMemory() override;
	EmulatedProcessor* GetProcessor() override;
	EmulatedSound* GetSound() override;

	// Machine info
	unsigned int GetFrameCount() override;
	unsigned int GetTickCount() override;
	unsigned int GetTicksPerSecond() override;
	unsigned int GetTicksUntilNextFrame() override;

	// Execution
	void Step() override;
	void RunToNextFrame() override;
	virtual void Run();
	virtual void Stop();

	// Persistence
	void SaveState(Archive& archive) override;
	void LoadState(Archive& archive) override;

	// Debugging
	void EnableBreakpoint(int address) override;
	void DisableBreakpoint(int address) override;

	// Gameboy

	// Creation
	static Gameboy* Create(const char* filename, Machine::Type machineType);
	static void Release(Gameboy* machine);

	// Application interface
	MachineToApplication* GetApplicationInterface();

	// Gameboy Components (non-virtual functions, direct concrete types)
	Cpu* GetGbCpu();
	Memory* GetGbMemory();
	Display* GetGbDisplay();
	Input* GetGbInput();
	Sound* GetGbSound();

	// Execution
	void RunDuringInstruction(
		unsigned int ticks);  ///< Should only be called by the CPU.  Won't do anything if called externally.

	// Double-speed mode (CGB only)
	bool IsDoubleSpeed();
	void SetDoubleSpeed(bool doubleSpeed);

protected:
	Gameboy(Memory* memory, Machine::Type machineType);
	virtual ~Gameboy();
	void Initialize();

	void InternalStep();  ///< Non-virtual Step.

	virtual void Serialize(Archive& archive);

	Machine::Type m_machineType;

	MachineToApplication* m_applicationInterface;

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
	unsigned int m_subInstructionTicksSpent;  ///< Tracks how many ticks were used during instruction execution

	bool m_doubleSpeed;

	std::list<ApplicationEvent> m_applicationEvents;
	std::list<ApplicationEvent>::iterator m_nextApplicationEvent;
	std::mutex m_applicationEventsLock;
	bool m_applicationEventsPending;
};

}  // namespace emunisce

#endif
