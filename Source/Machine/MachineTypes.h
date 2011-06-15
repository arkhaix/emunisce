#ifndef MACHINETYPES_H
#define MACHINETYPES_H

#include "PlatformTypes.h"

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

typedef u32 DisplayPixel;	///<RGBA, where R is the least significant byte and A is the most significant byte.

inline DisplayPixel DisplayPixelFromRGBA(u8 r, u8 g, u8 b, u8 a = 255)
{
	return (DisplayPixel)((a << 24) | (b << 16) | (g << 8) | (r << 0));
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
	b = (u8)(pixel >> 16);
	g = (u8)(pixel >> 8);
	r = (u8)(pixel >> 0);
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

#endif
