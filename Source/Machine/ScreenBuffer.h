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
#ifndef SCREENBUFFER_H
#define SCREENBUFFER_H

#include "MachineTypes.h"

class ScreenBuffer
{
public:

	virtual ~ScreenBuffer();

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual DisplayPixel* GetPixels() = 0;
};

template<int TWidth, int THeight>
class TScreenBuffer : public ScreenBuffer
{
public:

	DisplayPixel Pixels[TWidth * THeight];

	inline DisplayPixel GetPixel(int x, int y)
	{
		if(x<0 || x>=TWidth || y<0 || y>=THeight)
			return (DisplayPixel)0;

		return Pixels[y*TWidth + x];
	}

	inline void SetPixel(int x, int y, DisplayPixel value)
	{
		if(x<0 || x>=TWidth || y<0 || y>=THeight)
			return;

		Pixels[y*TWidth+x] = value;
	}

	virtual int GetWidth()
	{
		return TWidth;
	}

	virtual int GetHeight()
	{
		return THeight;
	}

	virtual DisplayPixel* GetPixels()
	{
		return &Pixels[0];
	}
};

class DynamicScreenBuffer : public ScreenBuffer
{
public:

	DisplayPixel* Pixels;
	int Width;
	int Height;

	DynamicScreenBuffer(int width, int height);
	~DynamicScreenBuffer();

	virtual int GetWidth();
	virtual int GetHeight();

	virtual DisplayPixel* GetPixels();
};


#endif
