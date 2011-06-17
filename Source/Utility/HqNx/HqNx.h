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
#ifndef HQ4X_H
#define HQ4X_H


namespace Emunisce
{

class ScreenBuffer;

class HqNx
{
public:

	static ScreenBuffer* Hq2x(ScreenBuffer* originalScreen);
	static ScreenBuffer* Hq3x(ScreenBuffer* originalScreen);
	static ScreenBuffer* Hq4x(ScreenBuffer* originalScreen);

	static void Release(ScreenBuffer* buffer);
};

}	//namespace Emunisce

#endif
