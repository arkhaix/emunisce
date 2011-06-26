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
#ifndef IEMULATEDDISPLAY_H
#define IEMULATEDDISPLAY_H

#include "MachineTypes.h"



namespace Emunisce
{



struct ScreenResolution
{
	int width;
	int height;
};


class IEmulatedDisplay
{
public:

	virtual ScreenResolution GetScreenResolution() = 0;	///<Returns the native resolution of the screen.  Should be static per machine.
	virtual ScreenBuffer* GetStableScreenBuffer() = 0;	///<Returns the most recent screen buffer.  Note that the resolution may be different from GetScreenResolution, especially if filters are applied.
	virtual int GetScreenBufferCount() = 0;	///<Returns the id of the current screen buffer.  Not guaranteed to be unique or sequential, so use != when polling for changes.
};

}	//namespace Emunisce

#endif
