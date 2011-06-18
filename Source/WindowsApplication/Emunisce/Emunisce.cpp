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
#include "Emunisce.h"
using namespace Emunisce;

#include "windows.h"
#include "shlobj.h"
#include "shlwapi.h"

#include <string>

#include "PlatformIncludes.h"

#include "MachineIncludes.h"

#include "Serialization/SerializationIncludes.h"
#include "Serialization/FileSerializer.h"

#include "UserInterface.h"

#include "MachineRunner.h"
#include "ConsoleDebugger.h"

#include "../GdiPlusRenderer/GdiPlusRenderer.h"
#include "../KeyboardInput/KeyboardInput.h"
#include "../OpenGLRenderer/OpenGLRenderer.h"
#include "../WaveOutSound/WaveOutSound.h"


namespace Emunisce
{

class Phoenix_Private : public IWindowMessageListener
{
public:

	UserInterface* _UserInterface;

	Window* _Window;

	IEmulatedMachine* _Machine;
	IEmulatedMachine* _PendingMachine;

	MachineRunner* _Runner;
	ConsoleDebugger* _Debugger;

	//GdiPlusRenderer* _Renderer;
	OpenGLRenderer* _Renderer;
	KeyboardInput* _Input;
	WaveOutSound* _Sound;

	std::string _LastRomLoaded;

	bool _ShutdownRequested;

	Phoenix_Private()
	{
		_ShutdownRequested = false;

		_UserInterface = new UserInterface();

		_Window = new Window();

		_Machine = NULL;
		_PendingMachine = NULL;

		_Runner = new MachineRunner();
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
		_Runner->Shutdown();
		_UserInterface->Shutdown();

		delete _Sound;
		delete _Input;
		delete _Renderer;

		delete _Debugger;
		delete _Runner;

		if(_Machine)
		{
			MachineFactory::ReleaseMachine(_Machine);
		}

		delete _Window;

		delete _UserInterface;
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

	std::string GetBaseSaveStateFolder()
	{
		char path[MAX_PATH];
		SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, path);
		PathAppend(path, "Emunisce");
		PathAppend(path, "SaveStates");
		SHCreateDirectoryEx(NULL, path, NULL);
		return std::string(path);
	}

	std::string GetCurrentSaveStateFolder()
	{
		//todo: append machine name and game name
		return GetBaseSaveStateFolder();
	}

	std::string GetCurrentSaveStateFile(unsigned int id)
	{
		char file[MAX_PATH] = {0};

		std::string path = GetCurrentSaveStateFolder();
		strcpy_s(file, MAX_PATH, path.c_str());

		char idStr[MAX_PATH];
		sprintf_s(idStr, MAX_PATH, "%d.ess", id);

		PathAppend(file, idStr);

		return std::string(file);
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

}	//namespace Emunisce


EmunisceApplication::EmunisceApplication()
{
	m_private = new Phoenix_Private();

	m_private->_Window->Create(320, 240, "Emunisce", "Emunisce_RenderWindow");
	m_private->_Window->Show();
	m_private->_Window->SubscribeListener(m_private);

	m_private->_UserInterface->Initialize(this);
	m_private->_Runner->Initialize(this);
	m_private->_Debugger->Initialize(this);

	m_private->_Renderer->Initialize(this);
	m_private->_Input->Initialize(this);
	m_private->_Sound->Initialize(this);
}

EmunisceApplication::~EmunisceApplication()
{
	m_private->_Window->UnsubscribeListener(m_private);
	m_private->_Window->Destroy();

	delete m_private;
}

void EmunisceApplication::RunWindow()
{
	int lastFrameRendered = -1;

	while(ShutdownRequested() == false)
	{
		if(m_private->_PendingMachine != NULL)
		{
			IEmulatedMachine* newMachine = m_private->_PendingMachine;

			m_private->_Machine = newMachine;

			m_private->_UserInterface->SetMachine(newMachine);
			m_private->_Runner->SetMachine(newMachine);
			m_private->_Debugger->SetMachine(newMachine);

			m_private->_Renderer->SetMachine(newMachine);
			m_private->_Input->SetMachine(newMachine);
			m_private->_Sound->SetMachine(newMachine);

			m_private->AdjustWindowSize();

			m_private->_PendingMachine = NULL;
		}

		IEmulatedMachine* machine = GetMachine();
		if(machine)
		{
			if(m_private->_Renderer->GetLastFrameRendered() != machine->GetDisplay()->GetScreenBufferCount() && ShutdownRequested() == false)
			{
				HWND hwnd = (HWND)GetWindow()->GetHandle();
				RECT clientRect;

				GetClientRect(hwnd, &clientRect);
				InvalidateRect(hwnd, &clientRect, true);
				UpdateWindow(hwnd);

				GetWindow()->PumpMessages();
			}

			if(m_private->_Renderer->GetLastFrameRendered() == machine->GetDisplay()->GetScreenBufferCount() && ShutdownRequested() == false)
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

bool EmunisceApplication::LoadRom(const char* filename)
{
	//Prompt for a file if one wasn't provided
	char* selectedFilename = NULL;
	if(filename == NULL || strlen(filename) == 0)
	{
		bool fileSelected = m_private->_UserInterface->SelectFile(&selectedFilename);

		if(fileSelected == false)
			return false;

		filename = selectedFilename;
	}

	//Load it
	IEmulatedMachine* machine = MachineFactory::CreateMachine(filename);
	if(machine == NULL)
	{
		if(selectedFilename != NULL)
			free((void*)selectedFilename);

		return false;
	}

	//Successfully created a new Machine
	IEmulatedMachine* oldMachine = m_private->_Machine;

	bool wasPaused = m_private->_Runner->IsPaused();
	m_private->_Runner->Pause();

	//Let everyone know that the old one is going away
	NotifyMachineChanged(machine);

	if(wasPaused == false)
		m_private->_Runner->Run();

	//Release the old one
	if(oldMachine != NULL)
		MachineFactory::ReleaseMachine(oldMachine);

	//Remember the file used so we can reset later if necessary
	m_private->_LastRomLoaded = filename;

	if(selectedFilename != NULL)
		free((void*)selectedFilename);

	return true;
}

void EmunisceApplication::ResetRom()
{
	LoadRom(m_private->_LastRomLoaded.c_str());
}

void EmunisceApplication::SaveState(unsigned int id)
{
	std::string file = m_private->GetCurrentSaveStateFile(id);

	FileSerializer fs;
	fs.SetFile(file.c_str());

	Archive ar(&fs, ArchiveMode::Saving);

	bool wasPaused = m_private->_Runner->IsPaused();
	m_private->_Runner->Pause();

	m_private->_Machine->SaveState(ar);

	if(wasPaused == false)
		m_private->_Runner->Run();

	fs.CloseFile();
}

void EmunisceApplication::LoadState(unsigned int id)
{
	std::string file = m_private->GetCurrentSaveStateFile(id);

	FileSerializer fs;
	fs.SetFile(file.c_str());

	Archive ar(&fs, ArchiveMode::Loading);

	bool wasPaused = m_private->_Runner->IsPaused();
	m_private->_Runner->Pause();

	m_private->_Machine->LoadState(ar);

	if(wasPaused == false)
		m_private->_Runner->Run();

	fs.CloseFile();
}

void EmunisceApplication::NotifyMachineChanged(IEmulatedMachine* newMachine)
{
	//RunWindow must handle machine changes (rendering things have to happen on that thread)
	m_private->_PendingMachine = newMachine;
	while(m_private->_PendingMachine != NULL)
		Sleep(10);
}

IEmulatedMachine* EmunisceApplication::GetMachine()
{
	return m_private->_Machine;
}

bool EmunisceApplication::ShutdownRequested()
{
	return m_private->_ShutdownRequested;
}

void EmunisceApplication::RequestShutdown()
{
	m_private->_ShutdownRequested = true;

	if(m_private->_Window != NULL)
		m_private->_Window->RequestExit();
}

Window* EmunisceApplication::GetWindow()
{
	return m_private->_Window;
}

UserInterface* EmunisceApplication::GetUserInterface()
{
	return m_private->_UserInterface;
}

MachineRunner* EmunisceApplication::GetMachineRunner()
{
	return m_private->_Runner;
}

ConsoleDebugger* EmunisceApplication::GetDebugger()
{
	return m_private->_Debugger;
}

GdiPlusRenderer* EmunisceApplication::GetRenderer()
{
	return NULL;
	//return m_private->_Renderer;
}

KeyboardInput* EmunisceApplication::GetInput()
{
	return m_private->_Input;
}

WaveOutSound* EmunisceApplication::GetSound()
{
	return m_private->_Sound;
}
