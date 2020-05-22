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

#include "machine_types.h"

namespace emunisce {

namespace Machine {
typedef int Type;

enum {
	NoMachine, /* Was 'None', but that conflicts with something somewhere according to gcc */
	AutoSelect,

	Gameboy,
	GameboyColor,

	NumEmulatedMachines
};

#ifdef EmulatedMachine_ToString
static const char* ToString[] = {"None", "AutoSelect",

								 "Gameboy", "GameboyColor",

								 "NumEmulatedMachines"};
#endif
}  // namespace EmulatedMachine

class Archive;

struct ApplicationEvent {
	unsigned int eventId;
	unsigned int frameCount;
	unsigned int tickCount;

	bool operator<(const ApplicationEvent& rhs) {
		if (frameCount < rhs.frameCount)
			return true;
		else if (frameCount > rhs.frameCount)
			return false;

		return tickCount < rhs.tickCount;
	}
};

class EmulatedMachine {
public:
	// Machine type
	virtual Machine::Type GetType() = 0;
	virtual const char* GetRomTitle() = 0;

	// Application interface
	virtual void SetApplicationInterface(MachineToApplication* applicationInterface) = 0;
	virtual void AddApplicationEvent(
		ApplicationEvent& applicationEvent,
		bool relativeFrameCount =
			true) = 0;  ///< When this is called, the application is requesting a callback on
						///< IMachineToApplication::ApplicationEvent at the specified time (frameCount+tickCount).  If
						///< relativeFrameCount is true, then the specified frameCount is relative to the machine's
						///< current frameCount.  tickCount is always absolute (relative to the beginning of the frame;
						///< never the current tick count).  eventId should be stored and passed back to
						///< ApplicationEvent as-is.  eventId is non-sequential and potentially very large.
	virtual void RemoveApplicationEvent(unsigned int eventId) = 0;

	// Component access
	virtual EmulatedDisplay* GetDisplay() = 0;
	virtual EmulatedInput* GetInput() = 0;
	virtual EmulatedMemory* GetMemory() = 0;
	virtual EmulatedProcessor* GetProcessor() = 0;
	virtual EmulatedSound* GetSound() = 0;

	// Machine info
	virtual unsigned int GetFrameCount() = 0;
	virtual unsigned int GetTickCount() = 0;
	virtual unsigned int GetTicksPerSecond() = 0;
	virtual unsigned int GetTicksUntilNextFrame() = 0;

	// Execution
	virtual void Step() = 0;
	virtual void RunToNextFrame() = 0;

	// Persistence
	virtual void SaveState(Archive& archive) = 0;
	virtual void LoadState(Archive& archive) = 0;

	// Debugging
	virtual void EnableBreakpoint(int address) = 0;
	virtual void DisableBreakpoint(int address) = 0;
};

}  // namespace emunisce

#endif
