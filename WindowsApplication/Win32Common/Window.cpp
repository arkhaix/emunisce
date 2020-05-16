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
struct IUnknown; // combaseapi.h isn't compliant with /permissive- with Windows 8.1 SDK.
#include "Window.h"
using namespace Emunisce;

#include <algorithm>	///<std::find
#include <cstdio>


std::map<HWND, Window*> Window::m_hwndInstanceMap;
std::mutex Window::m_hwndInstanceMapLock;


Window::Window()
{
	m_needsDestroy = false;
	m_requestingExit = false;

	m_windowHandle = nullptr;

	m_size.width = 320;
	m_size.height = 240;

	m_position.x = CW_USEDEFAULT;
	m_position.y = CW_USEDEFAULT;
}

Window::~Window()
{
	if(m_needsDestroy == true)
		Destroy();
}


void Window::Create(int width, int height, const char* title, const char* className)
{
	m_size.width = width;
	m_size.height = height;

	WNDCLASS            wndClass;

	wndClass.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.lpfnWndProc    = Window::StaticWndProc;
	wndClass.cbClsExtra     = 0;
	wndClass.cbWndExtra     = 0;
	wndClass.hInstance      = nullptr;//hInstance;
	wndClass.hIcon          = LoadIcon(nullptr, IDI_APPLICATION);
	wndClass.hCursor        = LoadCursor(nullptr, IDC_ARROW);
	wndClass.hbrBackground  = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndClass.lpszMenuName   = nullptr;
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
		nullptr,                     // parent window handle
		nullptr,                     // window menu handle
		nullptr,//hInstance,                // program instance handle
		nullptr);                    // creation parameters

	if(m_windowHandle != nullptr)
	{
		{
			std::lock_guard<std::mutex> instanceMapLock(m_hwndInstanceMapLock);
			m_hwndInstanceMap[m_windowHandle] = this;
		}

		RECT windowRect;
		GetWindowRect(m_windowHandle, &windowRect);
		m_position.x = windowRect.left;
		m_position.y = windowRect.top;
		m_size.width = windowRect.right - windowRect.left;
		m_size.height = windowRect.bottom - windowRect.top;
	}
}

void Window::Destroy()
{
	m_requestingExit = true;
	Sleep(15);

	CloseWindow(m_windowHandle);
	DestroyWindow(m_windowHandle);

	{
		std::lock_guard<std::mutex> instanceMapLock(m_hwndInstanceMapLock);
		auto iter = m_hwndInstanceMap.find(m_windowHandle);
		if (iter != m_hwndInstanceMap.end())
			m_hwndInstanceMap.erase(iter);
	}

	m_windowHandle = nullptr;
	m_needsDestroy = false;
}


void Window::Show()
{
	ShowWindow(m_windowHandle, SW_SHOW);
}

void Window::Hide()
{
	ShowWindow(m_windowHandle, SW_HIDE);
}


void* Window::GetHandle()
{
	return (void*)m_windowHandle;
}


void Window::SubscribeListener(IWindowMessageListener* listener)
{
	std::lock_guard<std::recursive_mutex> scopedLock(m_listenersLock);

	auto iter = find(m_listeners.begin(), m_listeners.end(), listener);
	if(iter == m_listeners.end())
		m_listeners.push_back(listener);
}

void Window::UnsubscribeListener(IWindowMessageListener* listener)
{
	std::lock_guard<std::recursive_mutex> scopedLock(m_listenersLock);

	auto iter = find(m_listeners.begin(), m_listeners.end(), listener);
	if(iter == m_listeners.end())
		return;

	m_listeners.erase(iter);
}


void Window::PumpMessages()
{
	MSG msg;
	m_requestingExit = false;

	while(m_requestingExit == false && PeekMessage(&msg, m_windowHandle, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT)
		{
			std::lock_guard<std::recursive_mutex> scopedLock(m_listenersLock);
			for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
				(*iter)->Closed();

			break;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Window::RequestExit()
{
	m_requestingExit = true;
}


WindowSize Window::GetSize()
{
	return m_size;
}

void Window::SetSize(WindowSize size)
{
	m_size = size;
	MoveWindow(m_windowHandle, m_position.x, m_position.y, m_size.width, m_size.height, TRUE);
}


WindowPosition Window::GetPosition()
{
	return m_position;
}

void Window::SetPosition(WindowPosition position)
{
	m_position = position;
	MoveWindow(m_windowHandle, m_position.x, m_position.y, m_size.width, m_size.height, TRUE);
}


LRESULT CALLBACK Window::StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto iter = m_hwndInstanceMap.find(hWnd);
	if(iter == m_hwndInstanceMap.end())
		return DefWindowProc(hWnd, msg, wParam, lParam);

	Window* instance = iter->second;
	if(instance == nullptr)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	return instance->WndProc(hWnd, msg, wParam, lParam);
}

LRESULT CALLBACK Window::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(hWnd != m_windowHandle)
		return DefWindowProc(hWnd, msg, wParam, lParam);

	switch(msg)
	{

	case WM_PAINT:
		{
			std::lock_guard<std::recursive_mutex> scopedLock(m_listenersLock);
			for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
				(*iter)->Draw();

			ValidateRect(m_windowHandle, nullptr);

			return 0;
		}

	case WM_ERASEBKGND:
		return 0;

	case WM_KEYDOWN:
		{
			std::lock_guard<std::recursive_mutex> scopedLock(m_listenersLock);
			for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
				(*iter)->KeyDown((int)wParam);
			return 0;
		}

	case WM_KEYUP:
		{
			std::lock_guard<std::recursive_mutex> scopedLock(m_listenersLock);
			for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
				(*iter)->KeyUp((int)wParam);
			return 0;
		}

	case WM_SIZE:
		{
			std::lock_guard<std::recursive_mutex> scopedLock(m_listenersLock);

			RECT clientRect;
			GetClientRect(m_windowHandle, &clientRect);
			int newWidth = clientRect.right - clientRect.left;
			int newHeight = clientRect.bottom - clientRect.top;

			for(auto iter = m_listeners.begin(); iter != m_listeners.end(); ++iter)
				(*iter)->Resize(newWidth, newHeight);

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
