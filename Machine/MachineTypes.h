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
#ifndef MACHINETYPES_H
#define MACHINETYPES_H

#include "PlatformTypes.h"


namespace Emunisce
{

	//Application interface
	class IMachineToApplication;

	//Component forward-declarations
	class IEmulatedMachine;
	class IEmulatedDisplay;
	class IEmulatedInput;
	class IEmulatedMemory;
	class IEmulatedProcessor;
	class IEmulatedSound;

	//Display types
	class ScreenBuffer;
	class DynamicScreenBuffer;

	typedef u32 DisplayPixel;	///<ARGB, where B is the least significant byte and A is the most significant byte.

	inline DisplayPixel DisplayPixelFromRGBA(u8 r, u8 g, u8 b, u8 a = 255)
	{
		return (DisplayPixel)((a << 24) | (r << 16) | (g << 8) | (b << 0));
	}

	inline DisplayPixel DisplayPixelFromRGBA(float r, float g, float b, float a = 1.f)
	{
		u8 r8 = (u8)(r * 255.f);
		u8 g8 = (u8)(g * 255.f);
		u8 b8 = (u8)(b * 255.f);
		u8 a8 = (u8)(a * 255.f);
		return DisplayPixelFromRGBA(r8, g8, b8, a8);
	}

	inline void DisplayPixelToRGBA(DisplayPixel pixel, u8& r, u8& g, u8& b, u8&a)
	{
		a = (u8)(pixel >> 24);
		r = (u8)(pixel >> 16);
		g = (u8)(pixel >> 8);
		b = (u8)(pixel >> 0);
	}

	inline void DisplayPixelToRGBA(DisplayPixel pixel, float& r, float& g, float& b, float& a)
	{
		u8 r8, g8, b8, a8;
		DisplayPixelToRGBA(pixel, r8, g8, b8, a8);

		r = (float)r8 / 255.f;
		g = (float)g8 / 255.f;
		b = (float)b8 / 255.f;
		a = (float)a8 / 255.f;
	}

}	//namespace Emunisce

#endif
