#ifndef MACHINEFACTORY_H
#define MACHINEFACTORY_H

class IEmulatedMachine;

namespace EmulatedMachine
{
	typedef int Type;

	enum
	{
		AutoSelect = 0,

		Gameboy,
		GameboyColor,

		NumEmulatedMachines
	};

	static const char* ToString[] =
	{
		"AutoSelect",

		"Gameboy",
		"GameboyColor",

		"NumEmulatedMachines"
	};
}

class MachineFactory
{
public:

	static IEmulatedMachine* CreateMachine(const char* romFilename, EmulatedMachine::Type machineType = EmulatedMachine::AutoSelect);
	static void ReleaseMachine(IEmulatedMachine* machine);
};

#endif
