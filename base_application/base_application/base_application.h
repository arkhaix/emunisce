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

#include <string>

#include "machine_to_application.h"
#include "user_interface.h"

namespace emunisce {

class Archive;

class EmulatedMachine;

class MachineFeature;
class Gui;
class Rewinder;
class InputRecording;

class InputManager;
class MachineRunner;

class CommandTrie;

class BaseApplication : public IUserInterface, public MachineToApplication {
public:
	// BaseApplication

	BaseApplication();
	~BaseApplication();

	// Emulated machine
	virtual void NotifyMachineChanged(EmulatedMachine* newMachine);
	virtual EmulatedMachine* GetMachine();

	// Machine features
	virtual Gui* GetGui();
	virtual Rewinder* GetRewinder();
	virtual InputRecording* GetInputRecorder();

	// Utilities
	virtual InputManager* GetInputManager();
	virtual MachineRunner* GetMachineRunner();

	// Shutdown
	virtual bool ShutdownRequested();
	virtual void RequestShutdown();

	// IUserInterface

	// User to application

	// Rom
	bool LoadRom(const char* filename) override;
	void Reset() override;

	// Emulation
	void SetEmulationSpeed(float multiplier) override;

	void Run() override;
	void Pause() override;

	void StepInstruction() override;
	void StepFrame() override;

	// State
	void SaveState(const char* name) override;
	void LoadState(const char* name) override;

	// Gui
	void EnableBackgroundAnimation() override;
	void DisableBackgroundAnimation() override;

	// Display
	void SetDisplayFilter(DisplayFilter::Type displayFilter) override;
	void SetVsync(bool enabled) override = 0;

	// Input movie
	void StartRecordingInput() override;
	void StopRecordingInput() override;

	void PlayMovie() override;
	void StopMovie() override;

	void SaveMovie(const char* name) override;
	void LoadMovie(const char* name) override;

	void PlayMacro(bool loop) override;
	void StopMacro() override;

	void SaveMacro(const char* name) override;
	void LoadMacro(const char* name) override;

	// Application to user

	void DisplayStatusMessage(const char* message) override = 0;
	void DisplayImportantMessage(MessageType::Type messageType, const char* message) override = 0;
	PromptResult::Type DisplayPrompt(PromptType::Type promptType, const char* title, const char* message,
									 void** extraResult) override = 0;

	bool SelectFile(char** result, const char* fileMask = 0) override = 0;

	void ConsolePrint(const char* text) override = 0;

	// IMachineToApplication

	void HandleApplicationEvent(unsigned int eventId) override;

	void SaveRomData(const char* name, unsigned char* buffer, unsigned int bytes) override;

	unsigned int GetRomDataSize(const char* name) override = 0;
	void LoadRomData(const char* name, unsigned char* buffer, unsigned int bytes) override;

	// Console command framework

	typedef void (BaseApplication::*ConsoleCommandHandler)(const char* params);
	virtual void AddConsoleCommand(const char* command, ConsoleCommandHandler func, const char* helpText);

	virtual unsigned int NumConsoleCommands();
	virtual const char* GetConsoleCommand(unsigned int index);

	virtual bool ExecuteConsoleCommand(
		const char* command);  ///< Returns true if the command was successfully executed. False otherwise.

	virtual unsigned int NumPossibleCommands(const char* prefix);
	virtual const char* GetPossibleCommand(const char* prefix, unsigned int index);

protected:
	virtual Archive* OpenRomData(const char* name, bool saving) = 0;
	virtual void CloseRomData(Archive* archive) = 0;

	virtual Archive* OpenSavestate(const char* name, bool saving) = 0;
	virtual void CloseSavestate(Archive* archive) = 0;

	virtual Archive* OpenMovie(const char* name, bool saving) = 0;
	virtual void CloseMovie(Archive* archive) = 0;

	virtual Archive* OpenMacro(const char* name, bool saving) = 0;
	virtual void CloseMacro(Archive* archive) = 0;

	// Built-in console commands

	virtual void CommandHelp(const char* params);
	virtual void CommandQuit(const char* params);
	virtual void CommandLoad(const char* params);
	virtual void CommandPause(const char* params);
	virtual void CommandRun(const char* params);
	virtual void CommandSaveState(const char* params);
	virtual void CommandLoadState(const char* params);
	virtual void CommandSpeed(const char* params);
	virtual void CommandMute(const char* params);
	virtual void CommandDisplayFilter(const char* params);
	virtual void CommandVsync(const char* params);
	virtual void CommandBackground(const char* params);

	bool m_shutdownRequested;

	// Emulated machine
	MachineFeature* m_machine;
	EmulatedMachine* m_wrappedMachine;

	// Machine features
	Gui* m_gui;
	Rewinder* m_rewinder;
	InputRecording* m_inputRecorder;

	// Utilities
	InputManager* m_inputManager;
	MachineRunner* m_machineRunner;

	std::string m_lastRomLoaded;

	// Console commands
	struct ConsoleCommandInfo {
		std::string command;
		std::string helpText;
		ConsoleCommandHandler func;
	};
	static const unsigned int MaxConsoleCommands = 1024;
	ConsoleCommandInfo m_consoleCommands[MaxConsoleCommands];
	unsigned int m_numConsoleCommands;
	CommandTrie* m_commandTrie;
};

}  // namespace emunisce

#endif
