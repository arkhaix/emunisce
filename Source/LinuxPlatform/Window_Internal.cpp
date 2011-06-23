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
#include "Window_Internal.h"
using namespace Emunisce;

#include <algorithm>	///<std::find
#include <cstdio>



Window_Private::Window_Private()
{
	m_needsDestroy = false;
	m_requestingExit = false;

	m_size.width = 320;
	m_size.height = 240;

	m_position.x = 0;
	m_position.y = 0;
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
}

void Window_Private::Destroy()
{
	m_requestingExit = true;
}


void Window_Private::Show()
{
}

void Window_Private::Hide()
{
}


void* Window_Private::GetHandle()
{
	return (void*)NULL;
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
	m_requestingExit = false;
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
}


WindowPosition Window_Private::GetPosition()
{
	return m_position;
}

void Window_Private::SetPosition(WindowPosition position)
{
	m_position = position;
}
