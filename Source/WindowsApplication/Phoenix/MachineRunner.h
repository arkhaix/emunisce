
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
#ifndef MACHINERUNNER_H
#define MACHINERUNNER_H

#include "windows.h"	///<HANDLE

class Phoenix;
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

	void Initialize(Phoenix* phoenix);
	void Shutdown();

	void SetMachine(IEmulatedMachine* machine);


	// Machine runner

	virtual void SetEmulationSpeed(float speed);	///<1.0 = normal, 0.0 = stopped, 2.0 = twice normal, any negative value = unlimited

	virtual void Run();	///<Runs the machine at the speed defined by SetEmulationSpeed.
	virtual void Pause();	///<Pauses the machine.  Preserves the SetEmulationSpeed setting.
	virtual bool IsPaused();

	virtual void StepInstruction();	///<Pauses if necessary, then steps forward one cpu instruction.
	virtual void StepFrame();	///<Pauses if necessary, then steps forward 1/60th of a second.


protected:

	Phoenix* m_phoenix;
	IEmulatedMachine* m_machine;

	HANDLE m_runnerThread;
	static DWORD WINAPI StaticRunnerThread(LPVOID param);
	DWORD RunnerThread();

	void Synchronize();
	void ResetSynchronizationState();

	bool m_shutdownRequested;
	bool m_waitRequested;
	bool m_waiting;
	HANDLE m_waitEvent;

	StepMode::Type m_stepMode;

	float m_emulationSpeed;
	bool m_throttlingEnabled;

	struct SynchronizationInfo
	{
		LARGE_INTEGER CountsPerSecond;
		LARGE_INTEGER CountsPerFrame;

		LARGE_INTEGER RunStartTime;

		LARGE_INTEGER CurrentRealTime;
		LARGE_INTEGER CurrentMachineTime;

		LARGE_INTEGER ElapsedRealTime;
		LARGE_INTEGER ElapsedMachineTime;
	};

	SynchronizationInfo m_syncState;
};

#endif
