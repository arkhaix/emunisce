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
#ifndef MACHINERUNNER_H
#define MACHINERUNNER_H

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "platform_includes.h"

namespace emunisce {

class EmulatedMachine;

namespace StepMode {
typedef int Type;

enum {
	Instruction = 0,
	Frame,

	NumStepModes
};
}  // namespace StepMode

class MachineRunner {
public:
	MachineRunner();
	virtual ~MachineRunner() = default;

	// Application component

	void Initialize();
	void Shutdown();

	void SetMachine(EmulatedMachine* machine);

	// Machine runner

	virtual float GetEmulationSpeed();
	virtual void SetEmulationSpeed(float speed);  ///< 1.0 = normal, 0.5 = half normal, 2.0 = twice normal, any value
												  ///< less than or equal to 0 = no throttle (max speed)

	virtual void Run();    ///< Runs the machine at the speed defined by SetEmulationSpeed.
	virtual void Pause();  ///< Pauses the machine.  Preserves the SetEmulationSpeed setting.
	virtual bool IsPaused();

	virtual void StepInstruction();  ///< Pauses if necessary, then steps forward one cpu instruction.
	virtual void StepFrame();        ///< Pauses if necessary, then steps forward 1/60th of a second.

protected:
	EmulatedMachine* m_machine;

	std::thread m_runnerThread;
	int RunnerThread();

	void Synchronize();
	void ResetSynchronizationState();

	bool m_shutdownRequested;
	bool m_waitRequested;
	bool m_waiting;
	std::mutex m_waitMutex;
	std::condition_variable m_waitCondition;
	bool m_waitSignalled;

	StepMode::Type m_stepMode;

	float m_emulationSpeed;

	typedef std::chrono::steady_clock clock;
	struct SynchronizationInfo {
		float MillisecondsPerFrame;

		clock::time_point RunStartTime;

		clock::time_point CurrentRealTime;
		clock::time_point CurrentMachineTime;

		clock::duration ElapsedRealTime;
		clock::duration ElapsedMachineTime;
	};

	SynchronizationInfo m_syncState;
};

}  // namespace emunisce

#endif
