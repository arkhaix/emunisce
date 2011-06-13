/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Phoenix.h"

#include "../WindowsPlatform/Window.h"

#include "../Machine/Machine.h"

#include "ConsoleDebugger.h"
#include "../GdiPlusRenderer/GdiPlusRenderer.h"
#include "../KeyboardInput/KeyboardInput.h"
#include "../WaveOutSound/WaveOutSound.h"

class Phoenix_Private
{
public:

	Window* _Window;

	Machine* _Machine;

	ConsoleDebugger* _Debugger;
	GdiPlusRenderer* _Renderer;
	KeyboardInput* _Input;
	WaveOutSound* _Sound;

	bool _ShutdownRequested;

	Phoenix_Private()
	{
		_ShutdownRequested = false;

		_Window = new Window();

		_Machine = NULL;

		_Debugger = new ConsoleDebugger();
		_Renderer = new GdiPlusRenderer();
		_Input = new KeyboardInput();
		_Sound = new WaveOutSound();
	}

	~Phoenix_Private()
	{
		_ShutdownRequested = true;

		_Sound->Shutdown();
		_Input->Shutdown();
		_Renderer->Shutdown();
		_Debugger->Shutdown();

		delete _Sound;
		delete _Input;
		delete _Renderer;
		delete _Debugger;

		if(_Machine)
		{
			Machine::Release(_Machine);
		}

		delete _Window;
	}
};

Phoenix::Phoenix()
{
	m_private = new Phoenix_Private();

	m_private->_Window->Create(320, 240, "PhoenixGB", "PhoenixGB_RenderWindow");
	m_private->_Window->Show();

	m_private->_Debugger->Initialize(this);
	m_private->_Renderer->Initialize(this);
	m_private->_Input->Initialize(this);
	m_private->_Sound->Initialize(this);
}

Phoenix::~Phoenix()
{
	m_private->_Window->Destroy();

	delete m_private;
}

void Phoenix::NotifyMachineChanged(Machine* newMachine)
{
	m_private->_Machine = newMachine;

	m_private->_Debugger->SetMachine(newMachine);
	m_private->_Renderer->SetMachine(newMachine);
	m_private->_Input->SetMachine(newMachine);
	m_private->_Sound->SetMachine(newMachine);
}

Machine* Phoenix::GetMachine()
{
	return m_private->_Machine;
}

bool Phoenix::ShutdownRequested()
{
	return m_private->_ShutdownRequested;
}

void Phoenix::RequestShutdown()
{
	m_private->_ShutdownRequested = true;
}

Window* Phoenix::GetWindow()
{
	return m_private->_Window;
}

ConsoleDebugger* Phoenix::GetDebugger()
{
	return m_private->_Debugger;
}

GdiPlusRenderer* Phoenix::GetRenderer()
{
	return m_private->_Renderer;
}

KeyboardInput* Phoenix::GetInput()
{
	return m_private->_Input;
}

WaveOutSound* Phoenix::GetSound()
{
	return m_private->_Sound;
}
