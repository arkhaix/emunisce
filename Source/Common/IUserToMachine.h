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
#ifndef IUSERTOMACHINE_H
#define IUSERTOMACHINE_H

class IUserToMachine
{
public:

	//Rom

	virtual bool LoadRom(const char* filename) = 0;	///<Attempts to load the specified rom.  Returns true on success, false on failure.

	virtual void Reset() = 0;	///<Reloads the last successfully loaded rom and resets the machine state to the beginning.


	//Emulation

	virtual void SetEmulationSpeed(float speed) = 0;	///<1.0 = normal, 0.0 = stopped, 2.0 = twice normal, any negative value = unlimited

	virtual void Run() = 0;	///<Runs the machine at the speed defined by SetEmulationSpeed.
	virtual void Pause() = 0;	///<Pauses the machine.  Preserves the SetEmulationSpeed setting.

	virtual void StepInstruction() = 0;	///<Pauses if necessary, then steps forward one cpu instruction.
	virtual void StepFrame() = 0;	///<Pauses if necessary, then steps forward 1/60th of a second.
};

#endif
