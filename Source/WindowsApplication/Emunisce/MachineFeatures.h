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
#ifndef MACHINEFEATURES_H
#define MACHINEFEATURES_H

#include "MachineIncludes.h"


namespace Emunisce
{

class MachineFeatures : public IEmulatedMachine
{
public:

	// MachineFeatures

	MachineFeatures();
	~MachineFeatures();

	void SetMachine(IEmulatedMachine* wrappedMachine);


	// IEmulatedMachine

	//Machine type
	virtual EmulatedMachine::Type GetType();
	virtual const char* GetRomTitle();
	
	//Application interface
	virtual void SetApplicationInterface(IMachineToApplication* applicationInterface);

	//Component access
	virtual IEmulatedDisplay* GetDisplay();
	virtual IEmulatedInput* GetInput();
	virtual IEmulatedMemory* GetMemory();
	virtual IEmulatedProcessor* GetProcessor();
	virtual IEmulatedSound* GetSound();

	//Machine info
	virtual unsigned int GetFrameCount();
	virtual unsigned int GetTicksPerSecond();
	virtual unsigned int GetTicksUntilNextFrame();

	//Execution
	virtual void Step();
	virtual void RunToNextFrame();

	//Persistence
	virtual void SaveState(Archive& archive);
	virtual void LoadState(Archive& archive);

	//Debugging
	virtual void EnableBreakpoint(int address);
	virtual void DisableBreakpoint(int address);


private:

	IEmulatedMachine* m_wrappedMachine;
};

}	//namespace Emunisce

#endif
