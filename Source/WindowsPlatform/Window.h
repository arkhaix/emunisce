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
#ifndef WINDOW_H
#define WINDOW_H

#include "windows.h"

#include <list>
#include <map>
#include <set>
using namespace std;

#include "Window.h"
#include "Mutex.h"


namespace Emunisce
{

	class IWindowMessageListener
	{
	public:

		virtual void Closed() = 0;

		virtual void Draw() = 0;

		virtual void Resize(int newWidth, int newHeight) = 0;

		virtual void KeyDown(int key) = 0;
		virtual void KeyUp(int key) = 0;
	};

	struct WindowSize
	{
		int width;
		int height;
	};

	struct WindowPosition
	{
		int x;
		int y;
	};

	class Window
	{
	public:

		Window();
		~Window();

		void Create(int width, int height, const char* title, const char* className);
		void Destroy();

		void* GetHandle();

		void SubscribeListener(IWindowMessageListener* listener);
		void UnsubscribeListener(IWindowMessageListener* listener);

		void PumpMessages();
		void RequestExit();

		void Show();
		void Hide();

		WindowSize GetSize();
		void SetSize(WindowSize size);

		WindowPosition GetPosition();
		void SetPosition(WindowPosition position);

	private:

		static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		static map<HWND, Window*> m_hwndInstanceMap;
		static Mutex m_hwndInstanceMapLock;

		LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		bool m_needsDestroy;
		bool m_requestingExit;

		HWND m_windowHandle;

		WindowSize m_size;
		WindowPosition m_position;

		list<IWindowMessageListener*> m_listeners;
		Mutex m_listenersLock;
	};

}	//namespace Emunisce

#endif
