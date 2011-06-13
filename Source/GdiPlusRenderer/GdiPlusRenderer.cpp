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
#include "GdiPlusRenderer.h"

//Windows
#include "windows.h"

//GdiPlus
#include "gdiplus.h"
using namespace Gdiplus;

//Platform
#include "../WindowsPlatform/Window.h"

//Machine
#include "../Machine/Types.h"
#include "../Machine/Machine.h"
#include "../Display/Display.h"

//Application
#include "../Phoenix/Phoenix.h"	///<todo: this is just here for requesting shutdown and getting a pointer to KeyboardInput.  Refactor this.
#include "../KeyboardInput/KeyboardInput.h"	///<todo: get rid of this


class GdiPlusRenderer_Private : public IWindowMessageListener
{
public:

	Phoenix* _Phoenix;
	
	Window* _Window;
	HWND _WindowHandle;

	Bitmap* _Bitmap;
	Color* _Palette[4];

	int _LastFrameRendered;

	GdiplusStartupInput _GdiplusStartupInput;
	ULONG_PTR _GdiplusToken;

	Machine* _Machine;

	GdiPlusRenderer_Private()
	{
		_Window = NULL;
		_WindowHandle = NULL;

		_LastFrameRendered = -1;

		_Machine = NULL;
	}

	~GdiPlusRenderer_Private()
	{
	}

	void InitializeGdiPlus()
	{
		GdiplusStartup(&_GdiplusToken, &_GdiplusStartupInput, NULL);

		_Bitmap = new Bitmap(160, 144, PixelFormat32bppARGB);

		_Palette[3] = new Color(0, 0, 0);
		_Palette[2] = new Color(85, 85, 85);
		_Palette[1] = new Color(170, 170, 170);
		_Palette[0] = new Color(255, 255, 255);
	}

	void ShutdownGdiPlus()
	{
		delete _Bitmap;

		for(int i=0;i<4;i++)
			delete _Palette[i];

		GdiplusShutdown(_GdiplusToken);
	}


	void AdjustWindowSize()
	{
		//Resize the window so that the client area is a whole multiple of 160x144

		RECT clientRect;
		GetClientRect(_WindowHandle, &clientRect);
		int clientWidth = clientRect.right - clientRect.left;
		int clientHeight = clientRect.bottom - clientRect.top;

		if( (clientWidth % 160) != 0 || (clientHeight % 144) != 0 )
		{
			//Figure out what the new client area should be
			// Adjust to the nearest multiple.
			// So, if we're at 161, we want to decrease to 160
			// But if we're at 319, we want to increase to 320

			int newWidth = clientWidth;
			if(clientWidth < 160)		///<Only support 1x scale or greater for now
				newWidth = 160;
			else if(clientWidth % 160 < 80)
				newWidth = clientWidth - (clientWidth % 160);	///<Nearest multiple is smaller than the current width
			else
				newWidth = clientWidth + (160 - (clientWidth % 160));	///<Nearest multiple is larger than the current width

			int newHeight = 144 * (newWidth / 160);


			//Figure out what we need to add to the original size in order to get our new target size

			int deltaWidth = newWidth - clientWidth;
			int deltaHeight = newHeight - clientHeight;


			//Apply the deltas to the window size
			// The client size is the drawable area
			// The window size is the drawable area plus the title bar, borders, etc

			RECT windowRect;
			GetWindowRect(_WindowHandle, &windowRect);
			windowRect.right += deltaWidth;
			windowRect.bottom += deltaHeight;

			MoveWindow(_WindowHandle, windowRect.left, windowRect.top, (windowRect.right - windowRect.left), (windowRect.bottom - windowRect.top), TRUE);
		}
	}


	void OnPaint()
	{
		//Check conditions

		if(_Machine == NULL)
			return;

		if(_LastFrameRendered == _Machine->GetFrameCount())
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
		if(_Machine == NULL || _Machine->GetDisplay() == NULL)
			return;

		Display* display = _Machine->GetDisplay();

		if(_LastFrameRendered == display->GetScreenBufferCount())
			return;

		ScreenBuffer screen = display->GetStableScreenBuffer();

		_LastFrameRendered = display->GetScreenBufferCount();

		BitmapData bitmapData;
		Gdiplus::Rect bitmapRect(0, 0, _Bitmap->GetWidth(), _Bitmap->GetHeight());
		Status lockResult = _Bitmap->LockBits(&bitmapRect, ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);
		if(lockResult != Ok)
			return;

		for(int y=0;y<144;y++)
		{
			UINT* pixel = (UINT*)bitmapData.Scan0;
			pixel += bitmapData.Stride * y / 4;

			for(int x=0;x<160;x++)
			{
				u8 screenPixel = screen.GetPixel(x,y);
				if(screenPixel > 3)
					continue;

				u8* argb = (u8*)pixel;
				argb[0] = _Palette[screenPixel]->GetB();
				argb[1] = _Palette[screenPixel]->GetG();
				argb[2] = _Palette[screenPixel]->GetR();
				argb[3] = _Palette[screenPixel]->GetA();

				pixel++;
			}
		}

		_Bitmap->UnlockBits(&bitmapData);
	}


	// IWindowMessageListener

	void Closed()
	{
		_Phoenix->RequestShutdown();
	}

	void Draw()
	{
		OnPaint();
	}

	void Resize()
	{
		AdjustWindowSize();
	}
	
	void KeyDown(int key)
	{
	}

	void KeyUp(int key)
	{
	}
};

void GdiPlusRenderer::Initialize(Phoenix* phoenix)
{
	m_private = new GdiPlusRenderer_Private();
	m_private->_Phoenix = phoenix;

	m_private->_Window = phoenix->GetWindow();
	m_private->_WindowHandle = (HWND)m_private->_Window->GetHandle();

	m_private->_Window->SubscribeListener(m_private);
	m_private->Resize();	///<Force an auto-resize on startup

	m_private->InitializeGdiPlus();
}

void GdiPlusRenderer::Shutdown()
{
	m_private->_Window->UnsubscribeListener(m_private);

	m_private->ShutdownGdiPlus();

	delete m_private;
}

void GdiPlusRenderer::SetMachine(Machine* machine)
{
	//todo: lock things to prevent crashing
	m_private->_Machine = machine;
}

HWND GdiPlusRenderer::GetTargetWindow()
{
	return m_private->_WindowHandle;
}
