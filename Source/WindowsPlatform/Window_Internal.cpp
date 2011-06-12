#include "Window_Internal.h"

#include <algorithm>	///<std::find


set<Window_Private*> Window_Private::m_validInstances;
Mutex Window_Private::m_validInstancesLock;


Window_Private::Window_Private()
{
	m_needsDestroy = false;

	m_handle = NULL;

	m_size.width = 640;
	m_size.height = 480;

	m_position.x = 0;
	m_position.y = 0;

	m_validInstancesLock.Acquire();
		m_validInstances.insert(this);
	m_validInstancesLock.Release();
}

Window_Private::~Window_Private()
{
	m_validInstancesLock.Acquire();
		m_validInstances.erase(this);
	m_validInstancesLock.Release();

	if(m_needsDestroy == true)
		Destroy();
}


void Window_Private::Create(int width, int height, const char* title, const char* className)
{
	m_size.width = width;
	m_size.height = height;

	WNDCLASS            wndClass;

	wndClass.style          = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc    = StaticWndProc;
	wndClass.cbClsExtra     = 0;
	wndClass.cbWndExtra     = 0;
	wndClass.hInstance      = NULL;//hInstance;
	wndClass.hIcon          = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName   = NULL;
	wndClass.lpszClassName  = className;

	RegisterClass(&wndClass);

	m_handle = CreateWindow(
		className,   // window class name
		title,  // window caption
		WS_OVERLAPPEDWINDOW,      // window style
		m_position.x,            // initial x position
		m_position.y,            // initial y position
		m_size.width,            // initial x size
		m_size.height,            // initial y size
		NULL,                     // parent window handle
		NULL,                     // window menu handle
		NULL,//hInstance,                // program instance handle
		(LPVOID)this);                    // creation parameters

	if(m_handle != NULL)
		m_needsDestroy = true;
}

void Window_Private::Destroy()
{
	DestroyWindow(m_handle);
	m_handle = NULL;
	m_needsDestroy = false;
}


void Window_Private::Show()
{
	ShowWindow(m_handle, SW_SHOW);
}

void Window_Private::Hide()
{
	ShowWindow(m_handle, SW_HIDE);
}


void* Window_Private::GetHandle()
{
	return (void*)m_handle;
}


void Window_Private::SubscribeListener(IWindowMessageListener* listener)
{
	auto iter = find(m_listeners.begin(), m_listeners.end(), listener);
	if(iter == m_listeners.end())
		m_listeners.push_back(listener);
}

void Window_Private::UnsubscribeListener(IWindowMessageListener* listener)
{
	auto iter = find(m_listeners.begin(), m_listeners.end(), listener);
	if(iter == m_listeners.end())
		return;

	m_listeners.erase(iter);
}


WindowSize Window_Private::GetSize()
{
	return m_size;
}

void Window_Private::SetSize(WindowSize size)
{
	m_size = size;
	MoveWindow(m_handle, m_position.x, m_position.y, m_size.width, m_size.height, TRUE);
}


WindowPosition Window_Private::GetPosition()
{
	return m_position;
}

void Window_Private::SetPosition(WindowPosition position)
{
	m_position = position;
	MoveWindow(m_handle, m_position.x, m_position.y, m_size.width, m_size.height, TRUE);
}


LRESULT CALLBACK Window_Private::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	CREATESTRUCT* creationParams = (CREATESTRUCT*)lParam;
	if(creationParams == NULL)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	Window_Private* instance = (Window_Private*)creationParams->lpCreateParams;
	if(instance == NULL)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	{
		ScopedMutex scopedLock(m_validInstancesLock);
		if(m_validInstances.find(instance) == m_validInstances.end())
			return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	return instance->WndProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window_Private::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(hWnd != m_handle)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	switch(msg)
	{
	case WM_PAINT:
		for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
			(*iter)->Draw();
	return 0;

	case WM_ERASEBKGND:
	return 0;

	case WM_KEYDOWN:
		for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
			(*iter)->KeyDown(wParam);
	return 0;

	case WM_KEYUP:
		for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
			(*iter)->KeyUp(wParam);
	return 0;

	case WM_SIZE:
		for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
			(*iter)->Resize();
	return 0;

	case WM_CLOSE:
	case WM_DESTROY:
		PostQuitMessage(0);
	return 0;
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}
