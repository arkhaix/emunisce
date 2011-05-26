#include "gdiPlusRenderer.h"

//Windows
#include "windows.h"

//GdiPlus
#include "gdiplus.h"
using namespace Gdiplus;

//Solution
#include "../common/types.h"
#include "../common/machine.h"
#include "../display/display.h"

//Project
#include "phoenix.h"
#include "keyboardInput.h"	///<todo: get rid of this

//Statics
GdiPlusRenderer* GdiPlusRenderer::m_defaultInstance = NULL;

class GdiPlusRenderer_Private
{
public:

	Phoenix* _Phoenix;
	
	HWND _Window;
	bool _CreatedWindow;

	Bitmap* _Bitmap;
	Color* _Palette[4];

	int _LastFrameRendered;

	GdiplusStartupInput _GdiplusStartupInput;
	ULONG_PTR _GdiplusToken;

	Machine* _Machine;

	GdiPlusRenderer_Private()
	{
		_Window = NULL;
		_CreatedWindow = false;

		_LastFrameRendered = -1;

		_Machine = NULL;
	}

	~GdiPlusRenderer_Private()
	{
	}

	void InitializeGdiPlus()
	{
		GdiplusStartup(&_GdiplusToken, &_GdiplusStartupInput, NULL);

		_Bitmap = new Bitmap(160, 144, PixelFormat24bppRGB);

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

	void CreateRendererWindow()
	{
		WNDCLASS            wndClass;

		wndClass.style          = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc    = GdiPlusRenderer::StaticWndProc;
		wndClass.cbClsExtra     = 0;
		wndClass.cbWndExtra     = 0;
		wndClass.hInstance      = NULL;//hInstance;
		wndClass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wndClass.lpszMenuName   = NULL;
		wndClass.lpszClassName  = TEXT("PhoenixGB");

		RegisterClass(&wndClass);

		_Window = CreateWindow(
			TEXT("PhoenixGB"),   // window class name
			TEXT("PhoenixGB"),  // window caption
			WS_OVERLAPPEDWINDOW,      // window style
			CW_USEDEFAULT,            // initial x position
			CW_USEDEFAULT,            // initial y position
			320,            // initial x size
			288,            // initial y size
			NULL,                     // parent window handle
			NULL,                     // window menu handle
			NULL,//hInstance,                // program instance handle
			NULL);                    // creation parameters

		if(_Window != NULL)
		{
			_CreatedWindow = true;

			ShowWindow(_Window, SW_SHOW);
			UpdateWindow(_Window);
		}
	}

	void DestroyRendererWindow()
	{
		if(_CreatedWindow == false)
			return;

		if(_Window == NULL)
			return;

		CloseWindow(_Window);
		DestroyWindow(_Window);

		_Window = NULL;
		_CreatedWindow = false;
	}

	void UseExistingWindow(HWND hWnd)
	{
		_Window = hWnd;
		_CreatedWindow = false;
		
		//todo: intercept WndProc
	}

	LRESULT WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if(hWnd != _Window)
			return DefWindowProc(hWnd, message, wParam, lParam);

		switch(message)
		{
		case WM_PAINT:
			OnPaint();
			return 0;
		case WM_ERASEBKGND:
			return 0;
		case WM_KEYDOWN:
			//todo: fix this
			if(_Phoenix->GetInput() != NULL)
				_Phoenix->GetInput()->KeyDown(wParam);
			return 0;
		case WM_KEYUP:
			//todo: fix this
			if(_Phoenix->GetInput() != NULL)
				_Phoenix->GetInput()->KeyUp(wParam);
			return 0;
		case WM_CLOSE:
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
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
		HDC hdc = BeginPaint(_Window, &paintStruct);

		Graphics graphics(hdc);

		RECT windowRect;
		GetClientRect(_Window, &windowRect);
		graphics.DrawImage(_Bitmap, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);


		//Draw the frame counter

		WCHAR strFrameCount[20];
		wsprintfW(strFrameCount, L"%d", _LastFrameRendered);

		Gdiplus::FontFamily someFontFamily(L"Consolas");
		Gdiplus::Font someFont(&someFontFamily, 16);
		Gdiplus::PointF someOrigin(0, 0);

		graphics.SetSmoothingMode(SmoothingModeAntiAlias);
		graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);

		StringFormat strFormat;
		GraphicsPath path;
		path.AddString(strFrameCount, wcslen(strFrameCount), &someFontFamily, 
			FontStyleRegular, 16, someOrigin, &strFormat );
		Pen pen(Color(0,0,0), 3);
		graphics.DrawPath(&pen, &path);
		SolidBrush brush(Color(255, 255, 255));
		graphics.FillPath(&brush, &path);

		EndPaint(_Window, &paintStruct);
	}

	void Render()
	{
		if(_Machine == NULL || _Machine->GetDisplay() == NULL)
			return;

		if(_LastFrameRendered == _Machine->GetFrameCount())
			return;

		Display* display = _Machine->GetDisplay();

		ScreenBuffer screen = display->GetStableScreenBuffer();

		_LastFrameRendered = _Machine->GetFrameCount();

		for(int y=0;y<144;y++)
			for(int x=0;x<160;x++)
				_Bitmap->SetPixel(x, y, *_Palette[ screen.GetPixel(x,y) ]);
	}
};

void GdiPlusRenderer::Initialize(Phoenix* phoenix, HWND window)
{
	m_defaultInstance = this;
	m_private = new GdiPlusRenderer_Private();
	m_private->_Phoenix = phoenix;

	if(window == NULL)
		m_private->CreateRendererWindow();
	else
		m_private->_Window = window;

	m_private->InitializeGdiPlus();
}

void GdiPlusRenderer::Shutdown()
{
	m_private->ShutdownGdiPlus();

	if(m_defaultInstance == this)
		m_defaultInstance = NULL;

	m_private->DestroyRendererWindow();

	delete m_private;
}

void GdiPlusRenderer::SetMachine(Machine* machine)
{
	//todo: lock things to prevent crashing
	m_private->_Machine = machine;
}

HWND GdiPlusRenderer::GetTargetWindow()
{
	return m_private->_Window;
}

void GdiPlusRenderer::RunMessagePump()
{
	if(m_private->_Window == NULL)
		return;

	MSG msg;

	while(m_private->_Phoenix->ShutdownRequested() == false)
	{
		while(PeekMessage(&msg, m_private->_Window, 0, 0, PM_REMOVE))
		{
			if(m_private->_Phoenix->ShutdownRequested() == true)
				break;

			if(msg.message == WM_QUIT)
			{
				m_private->_Phoenix->RequestShutdown();
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if(m_private->_Machine != NULL && m_private->_LastFrameRendered != m_private->_Machine->GetFrameCount())
		{
			RECT clientRect;
			GetClientRect(m_private->_Window, &clientRect);
			InvalidateRect(m_private->_Window, &clientRect, true);
			UpdateWindow(m_private->_Window);
		}

		Sleep(10);
	}
}

LRESULT CALLBACK GdiPlusRenderer::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(m_defaultInstance != NULL && m_defaultInstance->m_private != NULL)
		return m_defaultInstance->m_private->WndProc(hWnd, msg, wParam, lParam);

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
