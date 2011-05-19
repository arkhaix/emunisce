#include "windows.h"
#include "gdiplus.h"
using namespace Gdiplus;

#include <conio.h>

#include <iostream>
using namespace std;

#include "../cpu/cpu.h"
#include "../display/display.h"
#include "../memory/memory.h"

bool g_shutdownRequested = false;

Machine g_machine;

DWORD WINAPI EmulationThread(LPVOID param)
{
	g_machine._MachineType = MachineType::GameBoy;

	CPU cpu;
	g_machine._CPU = &cpu;

	Display display;
	g_machine._Display = &display;

	Memory* memory = Memory::CreateFromFile("C:/hg/Phoenix/Roms/test.gb");
	if(memory == NULL)
	{
		printf("Unsupported memory controller\n");
		return 1;
	}
	g_machine._Memory = memory;

	cpu.SetMachine(&g_machine);
	display.SetMachine(&g_machine);
	memory->SetMachine(&g_machine);

	memory->Initialize();
	cpu.Initialize();
	display.Initialize();

	int ticksPerFrame = 4194304;
	int frameTime = ticksPerFrame;

	while(g_shutdownRequested == false)
	{
		int ticks = cpu.Step();
		display.Run(ticks);

		frameTime -= ticks;
		if(frameTime <= 0)
		{
			g_machine._FrameCount++;
			frameTime += ticksPerFrame;
		}
	}

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

		ScreenBuffer screen = g_machine._Display->GetStableScreenBuffer();

		for(int y=0;y<144;y++)
			for(int x=0;x<160;x++)
				m_bitmap->SetPixel(x, y, *m_palette[ screen(x,y).Value ]);
	}

	Bitmap* GetBitmap()
	{
		return m_bitmap;
	}

private:

	Bitmap* m_bitmap;
	Color* m_palette[4];

	Machine* m_machine;
};

GdiPlusRenderer* g_renderer = NULL;

VOID OnPaint(HWND hwnd)
{
	static int lastFrameRendered = -1;

	if(g_renderer == NULL)
		return;

	if(lastFrameRendered == g_machine._FrameCount)
		return;

	g_renderer->Render();

	lastFrameRendered = g_machine._FrameCount;

	Bitmap* bitmap = g_renderer->GetBitmap();


	PAINTSTRUCT paintStruct;
	HDC hdc = BeginPaint(hwnd, &paintStruct);

	Graphics graphics(hdc);

	RECT windowRect;
	//GetWindowRect(hwnd, &windowRect);
	GetClientRect(hwnd, &windowRect);
	graphics.DrawImage(bitmap, 0, 0, windowRect.right - windowRect.left, windowRect.bottom - windowRect.top);
	//graphics.DrawImage(bitmap, 0, 0, 320, 288);

	WCHAR strFrameCount[20];
	wsprintfW(strFrameCount, L"%d", lastFrameRendered);

	Gdiplus::FontFamily someFontFamily(L"Consolas");
	Gdiplus::Font someFont(&someFontFamily, 16);
	Gdiplus::PointF someOrigin(0, 0);
	//Gdiplus::SolidBrush someBrush(Color(255, 0, 0));
	//graphics.DrawString(strFrameCount, -1, &someFont, someOrigin, &someBrush);

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

		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		InvalidateRect(hWnd, &clientRect, true);
		UpdateWindow(hWnd);

		Sleep(10);
	}
	

	WaitForSingleObject(emulationThreadHandle, 1000);

	g_renderer->Shutdown();
	delete g_renderer;

	GdiplusShutdown(gdiplusToken);
	return msg.wParam;
}  // WinMain

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
