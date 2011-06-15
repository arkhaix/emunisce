#include "ScreenBuffer.h"

#include "stdlib.h"	///<malloc, free


// ScreenBuffer

ScreenBuffer::~ScreenBuffer()
{
}


// DynamicScreenBuffer

DynamicScreenBuffer::DynamicScreenBuffer(int width, int height)
{
	Width = width;
	Height = height;
	Pixels = (DisplayPixel*)malloc(width * height * sizeof(DisplayPixel));
}

DynamicScreenBuffer::~DynamicScreenBuffer()
{
	free(Pixels);
}

int DynamicScreenBuffer::GetWidth()
{
	return Width;
}

int DynamicScreenBuffer::GetHeight()
{
	return Height;
}

DisplayPixel* DynamicScreenBuffer::GetPixels()
{
	return Pixels;
}
