/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

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
#include "Window.h"
using namespace Emunisce;

#include "Window_Internal.h"


Window::Window()
{
	m_private = new Window_Private();
}

Window::~Window()
{
	delete m_private;
}


void Window::Create(int width, int height, const char* title, const char* className)
{
	m_private->Create(width, height, title, className);
}

void Window::Destroy()
{
	m_private->Destroy();
}


void Window::Show()
{
	m_private->Show();
}

void Window::Hide()
{
	m_private->Hide();
}


WindowSize Window::GetSize()
{
	return m_private->GetSize();
}

void Window::SetSize(WindowSize size)
{
	m_private->SetSize(size);
}


WindowPosition Window::GetPosition()
{
	return m_private->GetPosition();
}

void Window::SetPosition(WindowPosition position)
{
	m_private->SetPosition(position);
}


void* Window::GetHandle()
{
	return m_private->GetHandle();
}


void Window::SubscribeListener(IWindowMessageListener* listener)
{
	m_private->SubscribeListener(listener);
}

void Window::UnsubscribeListener(IWindowMessageListener* listener)
{
	m_private->UnsubscribeListener(listener);
}


void Window::PumpMessages()
{
	m_private->PumpMessages();
}

void Window::RequestExit()
{
	m_private->RequestExit();
}
