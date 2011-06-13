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
#include "../OpenGLRenderer/OpenGLRenderer.h"
#include "../WaveOutSound/WaveOutSound.h"

class Phoenix_Private : public IWindowMessageListener
{
public:

	Window* _Window;

	Machine* _Machine;

	ConsoleDebugger* _Debugger;
	//GdiPlusRenderer* _Renderer;
	OpenGLRenderer* _Renderer;
	KeyboardInput* _Input;
	WaveOutSound* _Sound;

	bool _ShutdownRequested;

	Phoenix_Private()
	{
		_ShutdownRequested = false;

		_Window = new Window();

		_Machine = NULL;

		_Debugger = new ConsoleDebugger();
		//_Renderer = new GdiPlusRenderer();
		_Renderer = new OpenGLRenderer();
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


	void AdjustWindowSize()
	{
		//Resize the window so that the client area is a whole multiple of the display resolution

		if(_Machine == NULL)
			return;

		if(_Window == NULL)
			return;

		HWND windowHandle = (HWND)_Window->GetHandle();

		ScreenResolution resolution = _Machine->GetDisplay()->GetScreenResolution();
		int nativeWidth = resolution.width;
		int nativeHeight = resolution.height;

		RECT clientRect;
		GetClientRect(windowHandle, &clientRect);
		int clientWidth = clientRect.right - clientRect.left;
		int clientHeight = clientRect.bottom - clientRect.top;

		if( (clientWidth % nativeWidth) != 0 || (clientHeight % nativeHeight) != 0 )
		{
			//Figure out what the new client area should be.
			// Adjust to the nearest multiple.
			// Assuming the native width is 160...
			//  If we're at 161, we want to decrease to 160
			//  But if we're at 319, we want to increase to 320

			int newWidth = clientWidth;
			if(clientWidth < nativeWidth)		///<Only support 1x scale or greater for now
				newWidth = nativeWidth;
			else if(clientWidth % nativeWidth < 80)
				newWidth = clientWidth - (clientWidth % nativeWidth);	///<Nearest multiple is smaller than the current width
			else
				newWidth = clientWidth + (nativeWidth - (clientWidth % nativeWidth));	///<Nearest multiple is larger than the current width

			int newHeight = nativeHeight * (newWidth / nativeWidth);


			//Figure out what we need to add to the original size in order to get our new target size

			int deltaWidth = newWidth - clientWidth;
			int deltaHeight = newHeight - clientHeight;


			//Apply the deltas to the window size
			// The client size is the drawable area
			// The window size is the drawable area plus the title bar, borders, etc

			RECT windowRect;
			GetWindowRect(windowHandle, &windowRect);
			windowRect.right += deltaWidth;
			windowRect.bottom += deltaHeight;

			MoveWindow(windowHandle, windowRect.left, windowRect.top, (windowRect.right - windowRect.left), (windowRect.bottom - windowRect.top), TRUE);
		}
	}


	// IWindowMessageListener

	void Closed()
	{
		_ShutdownRequested = true;
	}

	void Draw()
	{
	}

	void Resize()
	{
		AdjustWindowSize();
	}
	
	void KeyDown(int key)
	{
	}

	void KeyUp(int key)
	{
	}
};

Phoenix::Phoenix()
{
	m_private = new Phoenix_Private();

	m_private->_Window->Create(320, 240, "PhoenixGB", "PhoenixGB_RenderWindow");
	m_private->_Window->Show();
	m_private->_Window->SubscribeListener(m_private);

	m_private->_Debugger->Initialize(this);
	m_private->_Renderer->Initialize(this);
	m_private->_Input->Initialize(this);
	m_private->_Sound->Initialize(this);
}

Phoenix::~Phoenix()
{
	m_private->_Window->UnsubscribeListener(m_private);
	m_private->_Window->Destroy();

	delete m_private;
}

void Phoenix::RunWindow()
{
	int lastFrameRendered = -1;

	while(ShutdownRequested() == false)
	{
		Machine* machine = GetMachine();
		if(machine)
		{
			while(m_private->_Renderer->GetLastFrameRendered() != machine->GetDisplay()->GetScreenBufferCount() && ShutdownRequested() == false)
			{
				HWND hwnd = (HWND)GetWindow()->GetHandle();
				RECT clientRect;

				GetClientRect(hwnd, &clientRect);
				InvalidateRect(hwnd, &clientRect, true);
				UpdateWindow(hwnd);
			}

			while(m_private->_Renderer->GetLastFrameRendered() == machine->GetDisplay()->GetScreenBufferCount() && ShutdownRequested() == false)
			{
				GetWindow()->PumpMessages();
				Sleep(15);
			}
		}
		else
		{
			GetWindow()->PumpMessages();
			Sleep(100);
		}
	}
}

void Phoenix::NotifyMachineChanged(Machine* newMachine)
{
	m_private->_Machine = newMachine;

	m_private->_Debugger->SetMachine(newMachine);
	m_private->_Renderer->SetMachine(newMachine);
	m_private->_Input->SetMachine(newMachine);
	m_private->_Sound->SetMachine(newMachine);

	m_private->AdjustWindowSize();
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

	if(m_private->_Window != NULL)
		m_private->_Window->RequestExit();
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
	return NULL;
	//return m_private->_Renderer;
}

KeyboardInput* Phoenix::GetInput()
{
	return m_private->_Input;
}

WaveOutSound* Phoenix::GetSound()
{
	return m_private->_Sound;
}
