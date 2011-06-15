#include "MachineFactory.h"

#include "Gameboy.h"

IEmulatedMachine* MachineFactory::CreateMachine(const char* romFilename, EmulatedMachine::Type machineType)
{
	return Gameboy::Create(romFilename);
}

void MachineFactory::ReleaseMachine(IEmulatedMachine* machine)
{
	Gameboy* gameboy = dynamic_cast<Gameboy*>(machine);
	if(gameboy != 0)
		Gameboy::Release(gameboy);
}
