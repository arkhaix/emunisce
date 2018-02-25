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
#ifndef BASEAPPLICATION_H
#define BASEAPPLICATION_H

#include "IUserInterface.h"

#include "IMachineToApplication.h"


namespace Emunisce
{

class Archive;

class IEmulatedMachine;

class MachineFeature;
class Gui;
class Rewinder;
class InputRecording;

class InputManager;
class MachineRunner;


class BaseApplication : public IUserInterface, public IMachineToApplication
{
public:

	// BaseApplication

	BaseApplication();
	~BaseApplication();

	//Emulated machine
	virtual void NotifyMachineChanged(IEmulatedMachine* newMachine);
	virtual IEmulatedMachine* GetMachine();

	//Machine features
	virtual Gui* GetGui();
	virtual Rewinder* GetRewinder();
	virtual InputRecording* GetInputRecorder();

	//Utilities
	virtual InputManager* GetInputManager();
	virtual MachineRunner* GetMachineRunner();

	//Shutdown
	virtual bool ShutdownRequested();
	virtual void RequestShutdown();


	// IUserInterface

	//User to application

	//Rom
	virtual bool LoadRom(const char* filename);
	virtual void Reset();

	//Emulation
	virtual void SetEmulationSpeed(float multiplier);

	virtual void Run();
	virtual void Pause();

	virtual void StepInstruction();
	virtual void StepFrame();

	//State
	virtual void SaveState(const char* name);
	virtual void LoadState(const char* name);

	//Gui
	virtual void EnableBackgroundAnimation();
	virtual void DisableBackgroundAnimation();

	//Display
	virtual void SetDisplayFilter(DisplayFilter::Type displayFilter);
	virtual void SetVsync(bool enabled) = 0;

	//Input movie
	virtual void StartRecordingInput();
	virtual void StopRecordingInput();

	virtual void PlayMovie();
	virtual void StopMovie();

	virtual void SaveMovie(const char* name);
	virtual void LoadMovie(const char* name);

	virtual void PlayMacro(bool loop);
	virtual void StopMacro();

	virtual void SaveMacro(const char* name);
	virtual void LoadMacro(const char* name);


	//Application to user

	virtual void DisplayStatusMessage(const char* message) = 0;
	virtual void DisplayImportantMessage(MessageType::Type messageType, const char* message) = 0;
	virtual PromptResult::Type DisplayPrompt(PromptType::Type promptType, const char* title, const char* message, void** extraResult) = 0;

	virtual bool SelectFile(char** result, const char* fileMask = 0) = 0;


	// IMachineToApplication

	virtual void HandleApplicationEvent(unsigned int eventId);

	virtual void SaveRomData(const char* name, unsigned char* buffer, unsigned int bytes);

	virtual unsigned int GetRomDataSize(const char* name) = 0;
	virtual void LoadRomData(const char* name, unsigned char* buffer, unsigned int bytes);


protected:

	virtual Archive* OpenRomData(const char* name, bool saving) = 0;
	virtual void CloseRomData(Archive* archive) = 0;

	virtual Archive* OpenSavestate(const char* name, bool saving) = 0;
	virtual void CloseSavestate(Archive* archive) = 0;

	virtual Archive* OpenMovie(const char* name, bool saving) = 0;
	virtual void CloseMovie(Archive* archive) = 0;

	virtual Archive* OpenMacro(const char* name, bool saving) = 0;
	virtual void CloseMacro(Archive* archive) = 0;


    //Console

    typedef void (BaseApplication::*ConsoleCommandHandler)(const char* command, const char* params);
    virtual void AddConsoleCommandHandler(const char* command, ConsoleCommandHandler func);
    
    virtual unsigned int NumConsoleCommands();
    virtual const char* GetConsoleCommand(unsigned int index);

    virtual void ExecuteConsoleCommand(const char* command);


	bool m_shutdownRequested;

	//Emulated machine
	MachineFeature* m_machine;
	IEmulatedMachine* m_wrappedMachine;

	//Machine features
	Gui* m_gui;
	Rewinder* m_rewinder;
	InputRecording* m_inputRecorder;

	//Utilities
	InputManager* m_inputManager;
	MachineRunner* m_machineRunner;

	char m_lastRomLoaded[1024];

    //Console commands
    struct ConsoleCommandInfo
    {
        char command[16];
        ConsoleCommandHandler func;
    };
    static const unsigned int MaxConsoleCommands = 1024;
    ConsoleCommandInfo m_consoleCommands[MaxConsoleCommands];
    unsigned int m_numConsoleCommands;
};

}	//namespace Emunisce

#endif
