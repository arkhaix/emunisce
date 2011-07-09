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
#ifndef MACHINEFACTORY_H
#define MACHINEFACTORY_H


namespace Emunisce
{

class IEmulatedMachine;

namespace EmulatedMachine
{
	typedef int Type;

	enum
	{
		None = 0,
		AutoSelect,

		Gameboy,
		GameboyColor,

		NumEmulatedMachines
	};

#ifdef EmulatedMachine_ToString
	static const char* ToString[] =
	{
		"None",
		"AutoSelect",

		"Gameboy",
		"GameboyColor",

		"NumEmulatedMachines"
	};
#endif
}

class MachineFactory
{
public:

	static IEmulatedMachine* CreateMachine(const char* romFilename, EmulatedMachine::Type machineType = EmulatedMachine::AutoSelect);
	static void ReleaseMachine(IEmulatedMachine* machine);
};

}	//namespace Emunisce

#endif
