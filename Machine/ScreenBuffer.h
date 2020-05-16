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
#ifndef SCREENBUFFER_H
#define SCREENBUFFER_H

#include <string.h>  ///<memcpy

#include "MachineTypes.h"

namespace Emunisce {

class Archive;

class ScreenBuffer {
   public:
	virtual ~ScreenBuffer();

	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual DisplayPixel* GetPixels() = 0;

	virtual void Clear(DisplayPixel clearColor) = 0;
	virtual ScreenBuffer* Clone() = 0;

	virtual void Serialize(Archive& archive);
};

template <int TWidth, int THeight>
class TScreenBuffer : public ScreenBuffer {
   public:
	DisplayPixel Pixels[TWidth * THeight];

	inline DisplayPixel GetPixel(int x, int y) {
		if (x < 0 || x >= TWidth || y < 0 || y >= THeight)
			return (DisplayPixel)0;

		return Pixels[y * TWidth + x];
	}

	inline void SetPixel(int x, int y, DisplayPixel value) {
		if (x < 0 || x >= TWidth || y < 0 || y >= THeight)
			return;

		Pixels[y * TWidth + x] = value;
	}

	int GetWidth() override { return TWidth; }

	int GetHeight() override { return THeight; }

	DisplayPixel* GetPixels() override { return &Pixels[0]; }

	void Clear(DisplayPixel clearColor) override {
		int numPixels = THeight * TWidth;
		for (int i = 0; i < numPixels; i++) Pixels[i] = clearColor;
	}

	ScreenBuffer* Clone() override {
		TScreenBuffer<TWidth, THeight>* result = new TScreenBuffer<TWidth, THeight>();
		memcpy(&result->Pixels[0], &Pixels[0], TWidth * THeight * sizeof(DisplayPixel));
		return result;
	}
};

class DynamicScreenBuffer : public ScreenBuffer {
   public:
	DisplayPixel* Pixels;
	int Width;
	int Height;

	DynamicScreenBuffer(int width, int height);
	~DynamicScreenBuffer() override;

	int GetWidth() override;
	int GetHeight() override;

	DisplayPixel* GetPixels() override;

	void Clear(DisplayPixel clearColor) override;
	ScreenBuffer* Clone() override;
};

}  // namespace Emunisce

#endif
