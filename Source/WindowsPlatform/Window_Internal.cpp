#include "Window_Internal.h"

#include <algorithm>	///<std::find
#include <cstdio>


map<HWND, Window_Private*> Window_Private::m_hwndInstanceMap;
Mutex Window_Private::m_hwndInstanceMapLock;


Window_Private::Window_Private()
{
	m_needsDestroy = false;
	m_requestingExit = false;

	m_windowHandle = NULL;

	m_size.width = 320;
	m_size.height = 240;

	m_position.x = CW_USEDEFAULT;
	m_position.y = CW_USEDEFAULT;
}

Window_Private::~Window_Private()
{
	if(m_needsDestroy == true)
		Destroy();
}


void Window_Private::Create(int width, int height, const char* title, const char* className)
{
	m_size.width = width;
	m_size.height = height;

	WNDCLASS            wndClass;

	wndClass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.lpfnWndProc    = Window_Private::StaticWndProc;
	wndClass.cbClsExtra     = 0;
	wndClass.cbWndExtra     = 0;
	wndClass.hInstance      = NULL;//hInstance;
	wndClass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName   = NULL;
	wndClass.lpszClassName  = className;

	RegisterClass(&wndClass);

	m_windowHandle = CreateWindow(
		className,   // window class name
		title,  // window caption
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,      // window style
		m_position.x,            // initial x position
		m_position.y,            // initial y position
		m_size.width,            // initial x size
		m_size.height,            // initial y size
		NULL,                     // parent window handle
		NULL,                     // window menu handle
		NULL,//hInstance,                // program instance handle
		NULL);                    // creation parameters

	if(m_windowHandle != NULL)
	{
		m_hwndInstanceMapLock.Acquire();
			m_hwndInstanceMap[m_windowHandle] = this;
		m_hwndInstanceMapLock.Release();
	}
}

void Window_Private::Destroy()
{
	m_requestingExit = true;
	Sleep(15);

	CloseWindow(m_windowHandle);
	DestroyWindow(m_windowHandle);

	m_hwndInstanceMapLock.Acquire();
		auto iter = m_hwndInstanceMap.find(m_windowHandle);
		if(iter != m_hwndInstanceMap.end())
			m_hwndInstanceMap.erase(iter);
	m_hwndInstanceMapLock.Release();

	m_windowHandle = NULL;
	m_needsDestroy = false;
}


void Window_Private::Show()
{
	ShowWindow(m_windowHandle, SW_SHOW);
}

void Window_Private::Hide()
{
	ShowWindow(m_windowHandle, SW_HIDE);
}


void* Window_Private::GetHandle()
{
	return (void*)m_windowHandle;
}


void Window_Private::SubscribeListener(IWindowMessageListener* listener)
{
	ScopedMutex scopedMutex(m_listenersLock);

	auto iter = find(m_listeners.begin(), m_listeners.end(), listener);
	if(iter == m_listeners.end())
		m_listeners.push_back(listener);
}

void Window_Private::UnsubscribeListener(IWindowMessageListener* listener)
{
	ScopedMutex scopedMutex(m_listenersLock);

	auto iter = find(m_listeners.begin(), m_listeners.end(), listener);
	if(iter == m_listeners.end())
		return;

	m_listeners.erase(iter);
}


void Window_Private::PumpMessages()
{
	MSG msg;
	m_requestingExit = false;

	while(m_requestingExit == false && PeekMessage(&msg, m_windowHandle, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT)
		{
			ScopedMutex scopedMutex(m_listenersLock);
			for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
				(*iter)->Closed();

			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window_Private::RequestExit()
{
	m_requestingExit = true;
}


WindowSize Window_Private::GetSize()
{
	return m_size;
}

void Window_Private::SetSize(WindowSize size)
{
	m_size = size;
	MoveWindow(m_windowHandle, m_position.x, m_position.y, m_size.width, m_size.height, TRUE);
}


WindowPosition Window_Private::GetPosition()
{
	return m_position;
}

void Window_Private::SetPosition(WindowPosition position)
{
	m_position = position;
	MoveWindow(m_windowHandle, m_position.x, m_position.y, m_size.width, m_size.height, TRUE);
}


LRESULT CALLBACK Window_Private::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto iter = m_hwndInstanceMap.find(hWnd);
	if(iter == m_hwndInstanceMap.end())
		return DefWindowProc(hWnd, msg, wParam, lParam);

	Window_Private* instance = iter->second;
	if(instance == NULL)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	return instance->WndProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window_Private::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(hWnd != m_windowHandle)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	switch(msg)
	{

	case WM_PAINT:
	{
		ScopedMutex scopedMutex(m_listenersLock);
		for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
			(*iter)->Draw();

		ValidateRect(m_windowHandle, NULL);

		return 0;
	}

	case WM_ERASEBKGND:
	return 0;

	case WM_KEYDOWN:
	{
		ScopedMutex scopedMutex(m_listenersLock);
		for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
			(*iter)->KeyDown(wParam);
		return 0;
	}

	case WM_KEYUP:
	{
		ScopedMutex scopedMutex(m_listenersLock);
		for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
			(*iter)->KeyUp(wParam);
		return 0;
	}

	case WM_SIZE:
	{
		ScopedMutex scopedMutex(m_listenersLock);
		for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
			(*iter)->Resize();
		return 0;
	}

	case WM_CLOSE:
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
