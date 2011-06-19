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
#include "MachineFeatures.h"
using namespace Emunisce;


// MachineFeatures

MachineFeatures::MachineFeatures()
{
	m_wrappedMachine = NULL;
}

MachineFeatures::~MachineFeatures()
{
}


void MachineFeatures::SetMachine(IEmulatedMachine* wrappedMachine)
{
	m_wrappedMachine = wrappedMachine;
}



// IEmulatedMachine

//Machine type
EmulatedMachine::Type MachineFeatures::GetType()
{
	if(m_wrappedMachine == NULL)
		return EmulatedMachine::None;

	return m_wrappedMachine->GetType();
}

const char* MachineFeatures::GetRomTitle()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetRomTitle();
}


//Application interface
void MachineFeatures::SetApplicationInterface(IMachineToApplication* applicationInterface)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->SetApplicationInterface(applicationInterface);
}


//Component access
IEmulatedDisplay* MachineFeatures::GetDisplay()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetDisplay();
}

IEmulatedInput* MachineFeatures::GetInput()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetInput();
}

IEmulatedMemory* MachineFeatures::GetMemory()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetMemory();
}

IEmulatedProcessor* MachineFeatures::GetProcessor()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetProcessor();
}

IEmulatedSound* MachineFeatures::GetSound()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetSound();
}


//Machine info
unsigned int MachineFeatures::GetFrameCount()
{
	if(m_wrappedMachine == NULL)
		return 0;

	return m_wrappedMachine->GetFrameCount();
}

unsigned int MachineFeatures::GetTicksPerSecond()
{
	if(m_wrappedMachine == NULL)
		return 1;

	return m_wrappedMachine->GetTicksPerSecond();
}

unsigned int MachineFeatures::GetTicksUntilNextFrame()
{
	if(m_wrappedMachine == NULL)
		return (unsigned int)-1;

	return m_wrappedMachine->GetTicksUntilNextFrame();
}


//Execution
void MachineFeatures::Step()
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->Step();
}

void MachineFeatures::RunOneFrame()
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->RunOneFrame();
}


//Persistence
void MachineFeatures::SaveState(Archive& archive)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->SaveState(archive);
}

void MachineFeatures::LoadState(Archive& archive)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->LoadState(archive);
}


//Debugging
void MachineFeatures::EnableBreakpoint(int address)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->EnableBreakpoint(address);
}

void MachineFeatures::DisableBreakpoint(int address)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->DisableBreakpoint(address);
}

