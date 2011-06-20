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
#include "MachineFeature.h"
using namespace Emunisce;


// MachineFeatures

MachineFeature::MachineFeature()
{
	m_wrappedMachine = NULL;
}

MachineFeature::~MachineFeature()
{
}


void MachineFeature::SetMachine(IEmulatedMachine* wrappedMachine)
{
	m_wrappedMachine = wrappedMachine;
}



// IEmulatedMachine

//Machine type
EmulatedMachine::Type MachineFeature::GetType()
{
	if(m_wrappedMachine == NULL)
		return EmulatedMachine::None;

	return m_wrappedMachine->GetType();
}

const char* MachineFeature::GetRomTitle()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetRomTitle();
}


//Application interface
void MachineFeature::SetApplicationInterface(IMachineToApplication* applicationInterface)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->SetApplicationInterface(applicationInterface);
}


//Component access
IEmulatedDisplay* MachineFeature::GetDisplay()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetDisplay();
}

IEmulatedInput* MachineFeature::GetInput()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetInput();
}

IEmulatedMemory* MachineFeature::GetMemory()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetMemory();
}

IEmulatedProcessor* MachineFeature::GetProcessor()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetProcessor();
}

IEmulatedSound* MachineFeature::GetSound()
{
	if(m_wrappedMachine == NULL)
		return NULL;

	return m_wrappedMachine->GetSound();
}


//Machine info
unsigned int MachineFeature::GetFrameCount()
{
	if(m_wrappedMachine == NULL)
		return 0;

	return m_wrappedMachine->GetFrameCount();
}

unsigned int MachineFeature::GetTicksPerSecond()
{
	if(m_wrappedMachine == NULL)
		return 1;

	return m_wrappedMachine->GetTicksPerSecond();
}

unsigned int MachineFeature::GetTicksUntilNextFrame()
{
	if(m_wrappedMachine == NULL)
		return (unsigned int)-1;

	return m_wrappedMachine->GetTicksUntilNextFrame();
}


//Execution
void MachineFeature::Step()
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->Step();
}

void MachineFeature::RunToNextFrame()
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->RunToNextFrame();
}


//Persistence
void MachineFeature::SaveState(Archive& archive)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->SaveState(archive);
}

void MachineFeature::LoadState(Archive& archive)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->LoadState(archive);
}


//Debugging
void MachineFeature::EnableBreakpoint(int address)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->EnableBreakpoint(address);
}

void MachineFeature::DisableBreakpoint(int address)
{
	if(m_wrappedMachine == NULL)
		return;

	m_wrappedMachine->DisableBreakpoint(address);
}

