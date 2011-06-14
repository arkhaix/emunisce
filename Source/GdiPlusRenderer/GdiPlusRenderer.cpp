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

#include "stdio.h"

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

		_Bitmap = NULL;
	}

	void ShutdownGdiPlus()
	{
		delete _Bitmap;

		GdiplusShutdown(_GdiplusToken);
	}


	void OnPaint()
	{
		//Check conditions

		if(_Machine == NULL)
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
		if(_Machine == NULL || _Machine->GetDisplay() == NULL)
			return;

		Display* display = _Machine->GetDisplay();

		if(_LastFrameRendered == display->GetScreenBufferCount())
			return;

		_LastFrameRendered = display->GetScreenBufferCount();


		ScreenBuffer* screen = display->GetStableScreenBuffer();

		int screenWidth = screen->GetWidth();
		int screenHeight = screen->GetHeight();

		if(_Bitmap == NULL || _Bitmap->GetWidth() != screenWidth || _Bitmap->GetHeight() != screenHeight)
		{
			delete _Bitmap;	///<Safe with NULL
			_Bitmap = new Bitmap(screenWidth, screenHeight, PixelFormat32bppARGB);
		}

		BitmapData bitmapData;
		Gdiplus::Rect bitmapRect(0, 0, screenWidth, screenHeight);
		Status lockResult = _Bitmap->LockBits(&bitmapRect, ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);
		if(lockResult != Ok)
			return;

		for(int y=0;y<screenHeight;y++)
		{
			UINT* pixel = (UINT*)bitmapData.Scan0;
			pixel += bitmapData.Stride * y / 4;

			for(int x=0;x<screenWidth;x++)
			{
				DisplayPixel screenPixel = screen->GetPixels()[y * screenWidth + x];

				*pixel = (UINT)screenPixel;

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


int GdiPlusRenderer::GetLastFrameRendered()
{
	return m_private->_LastFrameRendered;
}
