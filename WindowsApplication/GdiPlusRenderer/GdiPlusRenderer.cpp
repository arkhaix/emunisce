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
#undef NOMINMAX

#include "GdiPlusRenderer.h"
using namespace Emunisce;

//Windows
#include "windows.h"

//GdiPlus
#include "gdiplus.h"
using namespace Gdiplus;

#include "stdio.h"

//Platform
#include "PlatformIncludes.h"

//Machine
#include "MachineIncludes.h"

//Application
#include "../Emunisce/Emunisce.h"	///<todo: this is just here for requesting shutdown.  Refactor this.



namespace Emunisce
{

class GdiPlusRenderer_Private
{
public:

	EmunisceApplication* _Phoenix;

	HWND _WindowHandle;

	Bitmap* _Bitmap;

	int _LastFrameRendered;

	GdiplusStartupInput _GdiplusStartupInput;
	ULONG_PTR _GdiplusToken;

	IEmulatedMachine* _Machine;

	GdiPlusRenderer_Private()
	{
		_Phoenix = nullptr;
		_WindowHandle = nullptr;
		_Bitmap = nullptr;

		_LastFrameRendered = -1;

		_Machine = nullptr;
	}

	~GdiPlusRenderer_Private()
	{
	}

	void InitializeGdiPlus()
	{
		GdiplusStartup(&_GdiplusToken, &_GdiplusStartupInput, nullptr);

		_Bitmap = nullptr;
	}

	void ShutdownGdiPlus()
	{
		delete _Bitmap;

		GdiplusShutdown(_GdiplusToken);
	}


	void OnPaint()
	{
		//Check conditions

		if(_Machine == nullptr)
			return;

		if(_LastFrameRendered == _Machine->GetDisplay()->GetScreenBufferCount())
			return;


		//Render the frame

		Render();


		//Paint the rendered frame to the window

		PAINTSTRUCT paintStruct;
		HDC hdc = BeginPaint(_WindowHandle, &paintStruct);

		Graphics graphics(hdc);
		graphics.SetInterpolationMode(InterpolationModeNearestNeighbor);	///<Disable antialiasing
		graphics.SetSmoothingMode(SmoothingModeHighSpeed);	///<Dunno if this does anything useful
		graphics.SetPixelOffsetMode(PixelOffsetModeHighSpeed);	///<Dunno if this does anything useful

		RECT clientRect;
		GetClientRect(_WindowHandle, &clientRect);
		graphics.DrawImage(_Bitmap, 0, 0, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

		EndPaint(_WindowHandle, &paintStruct);
	}

	void Render()
	{
		if(_Machine == nullptr || _Machine->GetDisplay() == nullptr)
			return;

		IEmulatedDisplay* display = _Machine->GetDisplay();

		if(_LastFrameRendered == display->GetScreenBufferCount())
			return;

		_LastFrameRendered = display->GetScreenBufferCount();


		ScreenBuffer* screen = display->GetStableScreenBuffer();

		int screenWidth = screen->GetWidth();
		int screenHeight = screen->GetHeight();

		if(_Bitmap == nullptr || (int)_Bitmap->GetWidth() != screenWidth || (int)_Bitmap->GetHeight() != screenHeight)
		{
			if(_Bitmap != nullptr)
				delete _Bitmap;

			_Bitmap = new Bitmap(screenWidth, screenHeight, PixelFormat32bppARGB);
		}

		BitmapData bitmapData;
		Gdiplus::Rect bitmapRect(0, 0, screenWidth, screenHeight);
		Status lockResult = _Bitmap->LockBits(&bitmapRect, ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);
		if(lockResult != Ok)
			return;

		for(int y=0;y<screenHeight;y++)
		{
			u32* pixel = (u32*)bitmapData.Scan0;
			pixel += bitmapData.Stride * y / 4;

			for(int x=0;x<screenWidth;x++)
			{
				DisplayPixel screenPixel = screen->GetPixels()[y * screenWidth + x];

				*pixel = (u32)screenPixel;

				pixel++;
			}
		}

		_Bitmap->UnlockBits(&bitmapData);
	}
};

}	//namespace Emunisce

void GdiPlusRenderer::Initialize(EmunisceApplication* phoenix, HWND windowHandle)
{
	m_private = new GdiPlusRenderer_Private();
	m_private->_Phoenix = phoenix;

	m_private->_WindowHandle = windowHandle;

	m_private->InitializeGdiPlus();
}

void GdiPlusRenderer::Shutdown()
{
	m_private->ShutdownGdiPlus();

	delete m_private;
}

void GdiPlusRenderer::SetMachine(IEmulatedMachine* machine)
{
	//todo: lock things to prevent crashing
	m_private->_Machine = machine;
}


int GdiPlusRenderer::GetLastFrameRendered()
{
	return m_private->_LastFrameRendered;
}


void GdiPlusRenderer::SetVsync(bool enabled)
{
	//No option for this in GDI+
}


void GdiPlusRenderer::Draw()
{
	m_private->OnPaint();
}
