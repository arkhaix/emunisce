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
#include "MachineRunner.h"

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
	m_throttlingEnabled = true;
}



// Application component

void MachineRunner::Initialize(Phoenix* phoenix)
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

void MachineRunner::SetEmulationSpeed(float speed)
{
	m_emulationSpeed = speed;
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
			m_machine->RunOneFrame();
			Synchronize();
		}
	}

	return 0;
}

void MachineRunner::Synchronize()
{
	if(m_throttlingEnabled == false)
		return;

	return;

	int syncsPerSecond = 20;

	LARGE_INTEGER performanceFrequency;
	LARGE_INTEGER countsPerSync;
	QueryPerformanceFrequency(&performanceFrequency);
	countsPerSync.QuadPart = performanceFrequency.QuadPart / (LONGLONG)syncsPerSecond;

	double countsPerMillisecond = performanceFrequency.QuadPart / 1000.0;
	double millisecondsPerCount = 1.0 / countsPerMillisecond;

	int ticksPerSecond = m_machine->GetTicksPerSecond();
	int ticksPerSync = ticksPerSecond / syncsPerSecond;
	int ticksPerFrame = ticksPerSecond / 60;	///<Machine frame (60fps), not display frame (59.7fps)

	int ticksUntilSync = ticksPerSync;

	LARGE_INTEGER syncPeriodStartCount;
	QueryPerformanceCounter(&syncPeriodStartCount);


	LARGE_INTEGER curCount;

	m_machine->RunOneFrame();

	ticksUntilSync -= ticksPerFrame;	
	if(ticksUntilSync <= 0)
	{
		do
		{
			QueryPerformanceCounter(&curCount);
			LONGLONG countsTooFast = countsPerSync.QuadPart - (curCount.QuadPart - syncPeriodStartCount.QuadPart);
			if(countsTooFast <= 0)
				break;
			double millisecondsTooFast = millisecondsPerCount * countsTooFast;
			Sleep((DWORD)millisecondsTooFast);
		} while(curCount.QuadPart - syncPeriodStartCount.QuadPart < countsPerSync.QuadPart);

		syncPeriodStartCount.QuadPart = curCount.QuadPart;
		ticksUntilSync += ticksPerSync;
	}
}
