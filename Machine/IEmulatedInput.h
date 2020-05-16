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
#ifndef IEMULATEDINPUT_H
#define IEMULATEDINPUT_H

namespace Emunisce {

class IEmulatedInput {
   public:
	// Digital buttons

	virtual unsigned int NumButtons() = 0;
	virtual const char* GetButtonName(unsigned int index) = 0;

	virtual void ButtonDown(unsigned int index) = 0;
	virtual void ButtonUp(unsigned int index) = 0;

	virtual bool IsButtonDown(unsigned int index) = 0;
};

}  // namespace Emunisce

#endif
