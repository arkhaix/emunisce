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
#ifndef IEMULATEDMACHINE_H
#define IEMULATEDMACHINE_H

#include "MachineTypes.h"


namespace Emunisce
{

class Archive;

class IEmulatedMachine
{
public:

	//Machine type
	virtual EmulatedMachine::Type GetType() = 0;
	virtual const char* GetRomTitle() = 0;
	
	//Application interface
	virtual void SetApplicationInterface(IMachineToApplication* applicationInterface) = 0;

	//Component access
	virtual IEmulatedDisplay* GetDisplay() = 0;
	virtual IEmulatedInput* GetInput() = 0;
	virtual IEmulatedMemory* GetMemory() = 0;
	virtual IEmulatedProcessor* GetProcessor() = 0;
	virtual IEmulatedSound* GetSound() = 0;

	//Machine info
	virtual unsigned int GetFrameCount() = 0;
	virtual unsigned int GetTicksPerSecond() = 0;
	virtual unsigned int GetTicksUntilNextFrame() = 0;

	//Execution
	virtual void Step() = 0;
	virtual void RunOneFrame() = 0;

	//Persistence
	virtual void SaveState(Archive& archive) = 0;
	virtual void LoadState(Archive& archive) = 0;

	//Debugging
	virtual void EnableBreakpoint(int address) = 0;
	virtual void DisableBreakpoint(int address) = 0;
};

}	//namespace Emunisce

#endif
