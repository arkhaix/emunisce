#include "HqNx.h"

#include <stdlib.h>

extern void hq2x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );
extern void hq3x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );
extern void hq4x_32( unsigned char * pIn, unsigned char * pOut, int Xres, int Yres, int BpL );


class HqHelper
{
public:

	static u16* ConvertToRgb15(ScreenBuffer* screen)
	{
		int originalWidth = screen->GetWidth();
		int originalHeight = screen->GetHeight();
		DisplayPixel* originalPixels = screen->GetPixels();

		u16* rgb15Screen = (u16*)malloc(originalWidth * originalHeight * sizeof(u16));

		for(int y=0;y<originalHeight;y++)
		{
			for(int x=0;x<originalWidth;x++)
			{
				int index = y * originalWidth + x;

				DisplayPixel originalPixel = originalPixels[index];

				u8 r, g, b, a;
				DisplayPixelToRGBA(originalPixel, r, g, b, a);

				r >>= 3;
				g >>= 2;
				b >>= 3;

				rgb15Screen[index] = (r<<11) | (g<<5) | b;
			}
		}

		return rgb15Screen;
	}

	static ScreenBuffer* HqConvert(ScreenBuffer* originalScreen, int scale)
	{
		if(scale < 2 || scale > 4)
			return originalScreen;

		int originalWidth = originalScreen->GetWidth();
		int originalHeight = originalScreen->GetHeight();

		u16* rgb15Screen = HqHelper::ConvertToRgb15(originalScreen);

		int newWidth = originalWidth * scale;
		int newHeight = originalHeight * scale;

		DynamicScreenBuffer* result = new DynamicScreenBuffer(newWidth, newHeight);
		DisplayPixel* newPixels = result->GetPixels();

		if(scale == 2)
			hq2x_32((unsigned char*)rgb15Screen, (unsigned char*)result->GetPixels(), originalWidth, originalHeight, newWidth*4);
		else if(scale == 3)
			hq3x_32((unsigned char*)rgb15Screen, (unsigned char*)result->GetPixels(), originalWidth, originalHeight, newWidth*4);
		else //scale == 4
			hq4x_32((unsigned char*)rgb15Screen, (unsigned char*)result->GetPixels(), originalWidth, originalHeight, newWidth*4);

		free(rgb15Screen);

		int disableAlpha = 0xff000000;
		for(int y=0;y<newHeight;y++)
			for(int x=0;x<newWidth;x++)
				newPixels[y * newWidth + x] |= disableAlpha;

		return result;
	}
};

ScreenBuffer* HqNx::Hq2x(ScreenBuffer* originalScreen)
{
	return HqHelper::HqConvert(originalScreen, 2);
}

ScreenBuffer* HqNx::Hq3x(ScreenBuffer* originalScreen)
{
	return HqHelper::HqConvert(originalScreen, 3);
}

ScreenBuffer* HqNx::Hq4x(ScreenBuffer* originalScreen)
{
	return HqHelper::HqConvert(originalScreen, 4);
}

void HqNx::Release(ScreenBuffer* buffer)
{
	DynamicScreenBuffer* rtBuffer = dynamic_cast<DynamicScreenBuffer*>(buffer);
	if(rtBuffer != NULL)
		delete rtBuffer;
}
