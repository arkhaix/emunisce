#ifndef IEMULATEDMACHINE_H
#define IEMULATEDMACHINE_H

#include "MachineTypes.h"

class IEmulatedMachine
{
public:

	//Machine type
	virtual EmulatedMachine::Type GetType() = 0;

	//Component access
	virtual IEmulatedDisplay* GetDisplay() = 0;
	virtual IEmulatedInput* GetInput() = 0;
	virtual IEmulatedMemory* GetMemory() = 0;
	virtual IEmulatedProcessor* GetProcessor() = 0;
	virtual IEmulatedSound* GetSound() = 0;

	//Machine info
	virtual unsigned int GetFrameCount() = 0;
	virtual unsigned int GetTicksPerSecond() = 0;

	//Execution
	virtual void Step() = 0;
	virtual void RunOneFrame() = 0;
	virtual void Run() = 0;
	virtual void Stop() = 0;

	//Persistence
	virtual bool SaveState(const char* filename) = 0;
	virtual bool LoadState(const char* filename) = 0;

	//Debugging
	virtual void EnableBreakpoint(int address) = 0;
	virtual void DisableBreakpoint(int address) = 0;
};

#endif
