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
#ifndef WINDOW_INTERNAL_H
#define WINDOW_INTERNAL_H

#include <list>
#include <map>
#include <set>
using namespace std;

#include "Window.h"
#include "Mutex.h"


namespace Emunisce
{

class Window_Private
{
public:

	Window_Private();
	~Window_Private();

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

	bool m_needsDestroy;
	bool m_requestingExit;

	WindowSize m_size;
	WindowPosition m_position;

	list<IWindowMessageListener*> m_listeners;
	Mutex m_listenersLock;
};

}	//namespace Emunisce

#endif
