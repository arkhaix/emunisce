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
#include "MachineRunner.h"
using namespace Emunisce;

#include "windows.h"

#include "MachineIncludes.h"


MachineRunner::MachineRunner()
{
	m_phoenix = NULL;
	m_machine = NULL;

	m_runnerThread = NULL;

	m_shutdownRequested = false;
	m_waitRequested = true;
	m_waiting = false;
	m_waitEvent = NULL;

	m_emulationSpeed = 1.f;

	QueryPerformanceFrequency(&m_syncState.CountsPerSecond);
	m_syncState.CountsPerFrame.QuadPart = m_syncState.CountsPerSecond.QuadPart / 60;	///<todo: this is bad. When CountsPerSecond is 1000, this will be 16.66667 rounded down to 16, throwing everything off by two-thirds of a millisecond per frame (40 milliseconds per second).
	ResetSynchronizationState();
}



// Application component

void MachineRunner::Initialize(EmunisceApplication* phoenix)
{
	m_waitEvent = CreateEvent(NULL, FALSE, TRUE, NULL);

	m_runnerThread = CreateThread(NULL, 0, StaticRunnerThread, (LPVOID)this, 0, NULL);
}

void MachineRunner::Shutdown()
{
	DWORD exitCode = STILL_ACTIVE;
	GetExitCodeThread(m_runnerThread, &exitCode);

	if(exitCode == STILL_ACTIVE)
	{
		Pause();

		m_shutdownRequested = true;

		m_waitRequested = false;	
		SetEvent(m_waitEvent);
		CloseHandle(m_waitEvent);

		WaitForSingleObject(m_runnerThread, 1000);
	}
}


void MachineRunner::SetMachine(IEmulatedMachine* machine)
{
	m_machine = machine;
}



// Machine runner

float MachineRunner::GetEmulationSpeed()
{
	return m_emulationSpeed;
}

void MachineRunner::SetEmulationSpeed(float multiplier)
{
	ResetSynchronizationState();
	m_emulationSpeed = multiplier;
}


void MachineRunner::Run()
{
	m_stepMode = StepMode::Frame;

	m_waitRequested = false;
	SetEvent(m_waitEvent);
}

void MachineRunner::Pause()
{
	m_waitRequested = true;
	while(m_waiting == false)
		Sleep(1);
}

bool MachineRunner::IsPaused()
{
	return m_waiting;
}


void MachineRunner::StepInstruction()
{
	Pause();

	m_stepMode = StepMode::Instruction;

	m_waitRequested = true;
	SetEvent(m_waitEvent);
}

void MachineRunner::StepFrame()
{
	Pause();

	m_stepMode = StepMode::Frame;

	m_waitRequested = true;
	SetEvent(m_waitEvent);
}



DWORD WINAPI MachineRunner::StaticRunnerThread(LPVOID param)
{
	MachineRunner* instance = (MachineRunner*)param;
	if(instance == NULL)
		return -1;

	return instance->RunnerThread();
}

DWORD MachineRunner::RunnerThread()
{
	while(true)
	{
		if(m_waitRequested == true)
		{
			m_waiting = true;
			WaitForSingleObject(m_waitEvent, INFINITE);
			ResetSynchronizationState();
			m_waiting = false;
		}

		if(m_shutdownRequested == true)
			break;

		if(m_machine == NULL)
		{
			Sleep(250);
			continue;
		}

		if(m_stepMode == StepMode::Instruction)
		{
			m_machine->Step();
		}
		else if(m_stepMode == StepMode::Frame)
		{
			m_machine->RunToNextFrame();
			Synchronize();
		}
	}

	return 0;
}

void MachineRunner::Synchronize()
{
	if(m_emulationSpeed < +1e-5)
		return;
		

	//Note: This function assumes it's being called 60 times per second (defined by CountsPerFrame).
	//		It will synchronize itself to that rate.

	QueryPerformanceCounter(&m_syncState.CurrentRealTime);
	m_syncState.CurrentMachineTime.QuadPart += (LONGLONG)((double)m_syncState.CountsPerFrame.QuadPart * (1.0 / (double)m_emulationSpeed));

	m_syncState.ElapsedRealTime.QuadPart = m_syncState.CurrentRealTime.QuadPart - m_syncState.RunStartTime.QuadPart;
	m_syncState.ElapsedMachineTime.QuadPart = m_syncState.CurrentMachineTime.QuadPart - m_syncState.RunStartTime.QuadPart;

	if(m_syncState.ElapsedRealTime.QuadPart > m_syncState.ElapsedMachineTime.QuadPart)
	{
		//Real time is ahead of the machine time, so the emulator is running too slow already.
		return;
	}

	LARGE_INTEGER timeDifference;
	timeDifference.QuadPart = m_syncState.ElapsedMachineTime.QuadPart - m_syncState.ElapsedRealTime.QuadPart;

	double secondsAhead = (double)timeDifference.QuadPart / (double)m_syncState.CountsPerSecond.QuadPart;
	int millisecondsAhead = (int)((secondsAhead * 1000.0) + 0.5);	///< +0.5 = Round up

	//The constant in the if-check below is arbitrary.
	//Using a small value (less than ~15 or so) may result in the OS sleeping for longer than expected 
	// due to the minimum timer resolution being higher than the specified sleep value.  If the emulator is running
	// fast enough to catch up (and if it doesn't cause too many thread context switches), then that's probably okay.
	//Using a high value (greater than ~50 or so) may result in noticeable jitter.
	if(millisecondsAhead >= 5)
	{
		Sleep(millisecondsAhead);
	}

	return;
}

void MachineRunner::ResetSynchronizationState()
{
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);

	m_syncState.RunStartTime = now;

	m_syncState.CurrentRealTime = now;
	m_syncState.CurrentMachineTime = now;

	m_syncState.ElapsedRealTime.QuadPart = 0;
	m_syncState.ElapsedMachineTime.QuadPart = 0;
}
