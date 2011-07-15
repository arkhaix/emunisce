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

#include "PlatformIncludes.h"


namespace Emunisce
{

class IEmulatedMachine;

namespace StepMode
{
	typedef int Type;

	enum
	{
		Instruction = 0,
		Frame,

		NumStepModes
	};
}


class MachineRunner
{
public:

	MachineRunner();


	// Application component

	void Initialize();
	void Shutdown();

	void SetMachine(IEmulatedMachine* machine);


	// Machine runner

	virtual float GetEmulationSpeed();
	virtual void SetEmulationSpeed(float speed);	///<1.0 = normal, 0.5 = half normal, 2.0 = twice normal, any value less than or equal to 0 = no throttle (max speed)

	virtual void Run();	///<Runs the machine at the speed defined by SetEmulationSpeed.
	virtual void Pause();	///<Pauses the machine.  Preserves the SetEmulationSpeed setting.
	virtual bool IsPaused();

	virtual void StepInstruction();	///<Pauses if necessary, then steps forward one cpu instruction.
	virtual void StepFrame();	///<Pauses if necessary, then steps forward 1/60th of a second.


protected:

	IEmulatedMachine* m_machine;

	class Thread_Runner : public Thread
	{
	public:

		virtual void EntryPoint(void* param)
		{
			MachineRunner* instance = (MachineRunner*)param;
			if(instance == NULL)
				return;

			instance->RunnerThread();
		}

		virtual void StopRequested() { /* Use MachineRunner::Shutdown() instead */ }
	};

	Thread_Runner m_runnerThread;
	int RunnerThread();

	void Synchronize();
	void ResetSynchronizationState();

	bool m_shutdownRequested;
	bool m_waitRequested;
	bool m_waiting;
	Event m_waitEvent;

	StepMode::Type m_stepMode;

	float m_emulationSpeed;

	struct SynchronizationInfo
	{
	    #if defined EMUNISCE_PLATFORM_WINDOWS
		LARGE_INTEGER CountsPerSecond;
		LARGE_INTEGER CountsPerFrame;

		LARGE_INTEGER RunStartTime;

		LARGE_INTEGER CurrentRealTime;
		LARGE_INTEGER CurrentMachineTime;

		LARGE_INTEGER ElapsedRealTime;
		LARGE_INTEGER ElapsedMachineTime;

		#elif defined EMUNISCE_PLATFORM_LINUX
		float MillisecondsPerFrame;

		Time RunStartTime;

		Time CurrentRealTime;
		Time CurrentMachineTime;

		Time ElapsedRealTime;
		Time ElapsedMachineTime;

		#endif
	};

	SynchronizationInfo m_syncState;
};

}	//namespace Emunisce

#endif
