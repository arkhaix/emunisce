#include "windows.h"
#include "gdiplus.h"
using namespace Gdiplus;

#include <conio.h>

#include <iostream>
using namespace std;

#include "../cpu/cpu.h"
#include "../display/display.h"
#include "../memory/memory.h"

#include "consoledebugger.h"

bool g_shutdownRequested = false;

Machine g_machine;

DWORD WINAPI EmulationThread(LPVOID param)
{
	ConsoleDebugger debugger;
	debugger.Run(&g_machine);

	g_shutdownRequested = true;

	return 0;
}

class GdiPlusRenderer
{
public:

	void Initialize(Machine* machine)
	{
		m_machine = machine;

		m_palette[3] = new Color(0, 0, 0);
		m_palette[2] = new Color(75, 75, 75);
		m_palette[1] = new Color(150, 150, 150);
		m_palette[0] = new Color(255, 255, 255);

		m_bitmap = new Bitmap(160, 144, PixelFormat24bppRGB);

		m_lastFrameRendered = -1;
	}

	void Shutdown()
	{
		delete m_bitmap;

		for(int i=0;i<4;i++)
			delete m_palette[i];
	}

	void Render()
	{
		if(m_machine == NULL || m_machine->_Display == NULL)
			return;

		if(m_machine->_FrameCount == m_lastFrameRendered)
			return;

		ScreenBuffer screen = g_machine._Display->GetStableScreenBuffer();

		m_lastFrameRendered = g_machine._FrameCount;

		for(int y=0;y<144;y++)
			for(int x=0;x<160;x++)
				m_bitmap->SetPixel(x, y, *m_palette[ screen.GetPixel(x,y) ]);
	}

	Bitmap* GetBitmap()
	{
		return m_bitmap;
	}

	int GetLastFrameRendered()
	{
		return m_lastFrameRendered;
	}

private:

	Bitmap* m_bitmap;
	Color* m_palette[4];

	int m_lastFrameRendered;

	Machine* m_machine;
};

GdiPlusRenderer* g_renderer = NULL;

VOID OnPaint(HWND hwnd)
{
	//Check conditions

	if(g_renderer == NULL)
		return;

	if(g_renderer->GetLastFrameRendered() == g_machine._FrameCount)
		return;


	//Render the frame

	g_renderer->Render();

	Bitmap* bitmap = g_renderer->GetBitmap();


	//Paint the rendered frame to the window

	PAINTSTRUCT paintStruct;
	HDC hdc = BeginPaint(hwnd, &paintStruct);

	Graphics graphics(hdc);

	RECT windowRect;
	GetClientRect(hwnd, &windowRect);
	graphics.DrawImage(bitmap, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);


	//Draw the frame counter

	WCHAR strFrameCount[20];
	wsprintfW(strFrameCount, L"%d", g_renderer->GetLastFrameRendered());

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

	EndPaint(hwnd, &paintStruct);
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	HWND                hWnd;
	MSG                 msg;
	WNDCLASS            wndClass;
	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;

	// Initialize GDI+.
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	wndClass.style          = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc    = WndProc;
	wndClass.cbClsExtra     = 0;
	wndClass.cbWndExtra     = 0;
	wndClass.hInstance      = hInstance;
	wndClass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName   = NULL;
	wndClass.lpszClassName  = TEXT("PhoenixGB");

	RegisterClass(&wndClass);

	hWnd = CreateWindow(
		TEXT("PhoenixGB"),   // window class name
		TEXT("PhoenixGB"),  // window caption
		WS_OVERLAPPEDWINDOW,      // window style
		CW_USEDEFAULT,            // initial x position
		CW_USEDEFAULT,            // initial y position
		320,            // initial x size
		288,            // initial y size
		NULL,                     // parent window handle
		NULL,                     // window menu handle
		hInstance,                // program instance handle
		NULL);                    // creation parameters

	ShowWindow(hWnd, iCmdShow);
	UpdateWindow(hWnd);


	g_renderer = new GdiPlusRenderer();
	g_renderer->Initialize(&g_machine);


	HANDLE emulationThreadHandle = CreateThread(NULL, 0, EmulationThread, NULL, 0, NULL);
	

	while(g_shutdownRequested == false)
	{
		while(PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
			{
				g_shutdownRequested = true;
				break;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		if(g_renderer && g_renderer->GetLastFrameRendered() != g_machine._FrameCount)
		{
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			InvalidateRect(hWnd, &clientRect, true);
			UpdateWindow(hWnd);
		}

		Sleep(10);
	}
	

	WaitForSingleObject(emulationThreadHandle, 1000);

	g_renderer->Shutdown();
	delete g_renderer;

	GdiplusShutdown(gdiplusToken);
	return msg.wParam;
}  // WinMain

int TestMemory(void)
{
	CPU cpu;
	Display display;
	Memory* memory = Memory::CreateFromFile("C:/hg/Phoenix/Roms/test.gb");

	Machine machine;
	machine._CPU = &cpu;
	machine._Display = &display;
	machine._Memory = memory;

	cpu.SetMachine(&machine);
	display.SetMachine(&machine);
	memory->SetMachine(&machine);

	volatile u8 data;
	printf("Going...\n");

	int startTime = GetTickCount();
	for(int i=0;i<10000;i++)
	{
		for(u16 address=0;address<0xffff;address++)
		{
			data = memory->Read8(address);
		}
	}
	int endTime = GetTickCount();

	printf("Time: %d\n", endTime - startTime);
	system("pause");
	return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, 
	WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_PAINT:
		OnPaint(hWnd);
		return 0;
	case WM_ERASEBKGND:
		return 0;
	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
} // WndProc
