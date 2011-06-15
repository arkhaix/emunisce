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
