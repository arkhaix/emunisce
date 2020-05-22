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
#include "base_application.h"
using namespace emunisce;

#include <algorithm>
#include <cstdlib>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "MachineIncludes.h"
#include "PlatformIncludes.h"
#include "command_trie.h"
#include "gui.h"
#include "input_manager.h"
#include "input_recording.h"
#include "machine_feature.h"
#include "machine_runner.h"
#include "rewinder.h"
#include "serialization/MemorySerializer.h"
#include "serialization/SerializationIncludes.h"

// BaseApplication

BaseApplication::BaseApplication() {
	// Properties

	m_shutdownRequested = false;

	// Machine features

	m_gui = new Gui();
	m_rewinder = new Rewinder();
	m_inputRecorder = new InputRecording();

	m_gui->SetComponentMachine(m_rewinder);
	m_rewinder->SetComponentMachine(m_inputRecorder);

	m_gui->SetFocus(true);

	m_machine = m_gui;
	m_wrappedMachine = nullptr;

	m_gui->SetApplication(this);
	m_rewinder->SetApplication(this);
	m_inputRecorder->SetApplication(this);

	// Utilities

	m_inputManager = new InputManager();
	m_inputManager->Initialize(this);

	m_machineRunner = new MachineRunner();
	m_machineRunner->Initialize();

	// Console commands
	m_commandTrie = new CommandTrie();
	m_numConsoleCommands = 0;
	AddConsoleCommand("help", &BaseApplication::CommandHelp, "Displays this help text");
	AddConsoleCommand("quit", &BaseApplication::CommandQuit, "Exits the application");
	AddConsoleCommand("load", &BaseApplication::CommandLoad, "Select a ROM to load");
	AddConsoleCommand("pause", &BaseApplication::CommandPause, "Pauses the game");
	AddConsoleCommand("run", &BaseApplication::CommandRun, "Resumes the game");
	AddConsoleCommand("savestate", &BaseApplication::CommandSaveState, "(name) Saves the current state to disk");
	AddConsoleCommand("loadstate", &BaseApplication::CommandLoadState, "(name) Loads the state from disk");
	AddConsoleCommand("speed", &BaseApplication::CommandSpeed,
					  "(speed) Sets the emulation speed. 1=normal, 0.5=half, 2=double, etc");
	AddConsoleCommand("displayfilter", &BaseApplication::CommandDisplayFilter,
					  "(filter) Sets the display filter. 1=normal, 2=hq2x, 3=hq3x, 4=hq4x");
	AddConsoleCommand("vsync", &BaseApplication::CommandVsync, "(on/off) Toggles vsync on or off");
	AddConsoleCommand("background", &BaseApplication::CommandBackground,
					  "(on/off) Toggles the background animation on or off");
}

BaseApplication::~BaseApplication() {
	m_shutdownRequested = true;

	m_machineRunner->Shutdown();
	delete m_machineRunner;

	// input manager doesn't need shutdown
	delete m_inputManager;

	delete m_inputRecorder;
	delete m_rewinder;
	delete m_gui;

	if (m_wrappedMachine) {
		MachineFactory::ReleaseMachine(m_wrappedMachine);
	}
}

// Emulated machine
void BaseApplication::NotifyMachineChanged(EmulatedMachine* newMachine) {
	if (newMachine != m_gui && newMachine != m_inputRecorder && newMachine != m_rewinder) {
		m_gui->SetFocus(false);
		m_machine->SetEmulatedMachine(newMachine);
		m_wrappedMachine = newMachine;
	}

	m_inputManager->SetMachine(m_machine);
	m_machineRunner->SetMachine(m_machine);

	newMachine->SetApplicationInterface(this);
}

EmulatedMachine* BaseApplication::GetMachine() {
	return m_machine;
}

// Machine features
Gui* BaseApplication::GetGui() {
	return m_gui;
}

Rewinder* BaseApplication::GetRewinder() {
	return m_rewinder;
}

InputRecording* BaseApplication::GetInputRecorder() {
	return m_inputRecorder;
}

InputManager* BaseApplication::GetInputManager() {
	return m_inputManager;
}

MachineRunner* BaseApplication::GetMachineRunner() {
	return m_machineRunner;
}

// Shutdown
bool BaseApplication::ShutdownRequested() {
	return m_shutdownRequested;
}

void BaseApplication::RequestShutdown() {
	m_shutdownRequested = true;
}

// IUserInterface

// User to application

// Rom
bool BaseApplication::LoadRom(const char* filename) {
	// Prompt for a file if one wasn't provided
	char* selectedFilename = nullptr;
	if (filename == nullptr || strlen(filename) == 0) {
		bool fileSelected = SelectFile(&selectedFilename);

		if (fileSelected == false) {
			return false;
		}

		filename = selectedFilename;
	}

	// Load it
	EmulatedMachine* machine = MachineFactory::CreateMachine(filename);
	if (machine == nullptr) {
		if (selectedFilename != nullptr) {
			free((void*)selectedFilename);
		}

		return false;
	}

	// Successfully created a new Machine
	EmulatedMachine* oldMachine = m_wrappedMachine;

	bool wasPaused = m_machineRunner->IsPaused();
	m_machineRunner->Pause();

	// Let everyone know that the old one is going away
	NotifyMachineChanged(machine);

	if (wasPaused == false) {
		m_machineRunner->Run();
	}

	// Release the old one
	if (oldMachine != nullptr) {
		MachineFactory::ReleaseMachine(oldMachine);
	}

	// Remember the file used so we can reset later if necessary
	m_lastRomLoaded = filename;

	if (selectedFilename != nullptr) {
		free((void*)selectedFilename);
	}

	return true;
}

void BaseApplication::Reset() {
	LoadRom(m_lastRomLoaded.c_str());
}

// Emulation
void BaseApplication::SetEmulationSpeed(float multiplier) {
	m_machineRunner->SetEmulationSpeed(multiplier);
}

void BaseApplication::Run() {
	m_machineRunner->Run();
}

void BaseApplication::Pause() {
	m_machineRunner->Pause();
}

void BaseApplication::StepInstruction() {
	m_machineRunner->StepInstruction();
}

void BaseApplication::StepFrame() {
	m_machineRunner->StepFrame();
}

// State
void BaseApplication::SaveState(const char* name) {
	Archive* archive = OpenSavestate(name, true);
	if (archive == nullptr) {
		return;
	}

	bool wasPaused = m_machineRunner->IsPaused();
	m_machineRunner->Pause();

	m_machine->SaveState(*archive);

	if (wasPaused == false) {
		m_machineRunner->Run();
	}

	CloseSavestate(archive);
}

void BaseApplication::LoadState(const char* name) {
	Archive* archive = OpenSavestate(name, false);
	if (archive == nullptr) {
		return;
	}

	bool wasPaused = m_machineRunner->IsPaused();
	m_machineRunner->Pause();

	m_machine->LoadState(*archive);

	if (wasPaused == false) {
		m_machineRunner->Run();
	}

	CloseSavestate(archive);
}

// Gui
void BaseApplication::EnableBackgroundAnimation() {
	if (m_gui != nullptr) {
		m_gui->EnableBackgroundAnimation();
	}
}

void BaseApplication::DisableBackgroundAnimation() {
	if (m_gui != nullptr) {
		m_gui->DisableBackgroundAnimation();
	}
}

// Display
void BaseApplication::SetDisplayFilter(DisplayFilter::Type displayFilter) {
	if (m_gui != nullptr) {
		m_gui->SetDisplayFilter(displayFilter);
	}
}

// Input movie
void BaseApplication::StartRecordingInput() {
	if (m_inputRecorder != nullptr) {
		m_inputRecorder->StartRecording();
	}
}

void BaseApplication::StopRecordingInput() {
	if (m_inputRecorder != nullptr) {
		m_inputRecorder->StopRecording();
	}
}

void BaseApplication::PlayMovie() {
	if (m_inputRecorder != nullptr) {
		m_inputRecorder->StartPlayback(true, true, false);
	}
}

void BaseApplication::StopMovie() {
	if (m_inputRecorder != nullptr) {
		m_inputRecorder->StopPlayback();
	}
}

void BaseApplication::SaveMovie(const char* name) {
	Archive* archive = OpenMovie(name, true);
	if (archive == nullptr) {
		return;
	}

	m_inputRecorder->SerializeMovie(*archive);

	CloseMovie(archive);
}

void BaseApplication::LoadMovie(const char* name) {
	Archive* archive = OpenMovie(name, false);
	if (archive == nullptr) {
		return;
	}

	m_inputRecorder->SerializeMovie(*archive);

	CloseMovie(archive);
}

void BaseApplication::PlayMacro(bool loop) {
	if (m_inputRecorder != nullptr) {
		m_inputRecorder->StartPlayback(false, false, loop);
	}
}

void BaseApplication::StopMacro() {
	if (m_inputRecorder != nullptr) {
		m_inputRecorder->StopPlayback();
	}
}

void BaseApplication::SaveMacro(const char* name) {
	Archive* archive = OpenMacro(name, true);
	if (archive == nullptr) {
		return;
	}

	m_inputRecorder->SerializeHistory(*archive);

	CloseMacro(archive);
}

void BaseApplication::LoadMacro(const char* name) {
	Archive* archive = OpenMacro(name, false);
	if (archive == nullptr) {
		return;
	}

	m_inputRecorder->SerializeHistory(*archive);

	CloseMacro(archive);
}

// IMachineToApplication

void BaseApplication::HandleApplicationEvent(unsigned int eventId) {
	if (eventId >= 0x01000000 && eventId < 0x02000000) {
		if (m_inputRecorder != nullptr) {
			m_inputRecorder->ApplicationEvent(eventId);
		}
	}
	else if (eventId >= 0x02000000 && eventId < 0x03000000) {
		if (m_rewinder != nullptr) {
			m_rewinder->ApplicationEvent(eventId);
		}
	}
}

void BaseApplication::SaveRomData(const char* title, unsigned char* buffer, unsigned int bytes) {
	Archive* archive = OpenRomData(title, true);
	if (archive == nullptr) {
		return;
	}

	archive->SerializeBuffer(buffer, bytes);

	CloseRomData(archive);
}

void BaseApplication::LoadRomData(const char* title, unsigned char* buffer, unsigned int bytes) {
	Archive* archive = OpenRomData(title, false);
	if (archive == nullptr) {
		return;
	}

	archive->SerializeBuffer(buffer, bytes);

	CloseRomData(archive);
}

void BaseApplication::AddConsoleCommand(const char* command, ConsoleCommandHandler func, const char* helpText) {
	if (m_numConsoleCommands >= MaxConsoleCommands) {
		return;
	}

	if (func == nullptr) {
		return;
	}

	std::string lowercaseCommand = command;
	transform(lowercaseCommand.begin(), lowercaseCommand.end(), lowercaseCommand.begin(), [](unsigned char c) -> char {
		return (char)::tolower(c);
	});

	m_consoleCommands[m_numConsoleCommands].command = lowercaseCommand;
	m_consoleCommands[m_numConsoleCommands].helpText = helpText;
	m_consoleCommands[m_numConsoleCommands].func = func;

	m_numConsoleCommands++;

	m_commandTrie->Add(command);
}

unsigned int BaseApplication::NumConsoleCommands() {
	return m_numConsoleCommands;
}

const char* BaseApplication::GetConsoleCommand(unsigned int index) {
	if (index >= m_numConsoleCommands) {
		return nullptr;
	}

	return m_consoleCommands[index].command.c_str();
}

std::vector<std::string> SplitCommand(std::string command) {
	std::regex regex("[ \t\n]");
	std::vector<std::string> result(std::sregex_token_iterator(command.begin(), command.end(), regex, -1),
									std::sregex_token_iterator());

	return result;
}

bool BaseApplication::ExecuteConsoleCommand(const char* command) {
	std::vector<std::string> splitCommand = SplitCommand(command);
	if (splitCommand.size() < 1) {
		return false;
	}

	const char* commandName = splitCommand[0].c_str();
	std::string lowercaseCommandName = commandName;
	transform(lowercaseCommandName.begin(), lowercaseCommandName.end(), lowercaseCommandName.begin(),
			  [](unsigned char c) -> char {
				  return (char)::tolower(c);
			  });
	ConsolePrint(lowercaseCommandName.c_str());
	ConsolePrint("\n");

	// Resolve to a full command name via the prefix tree
	CommandTrie* node = m_commandTrie->GetNode(lowercaseCommandName.c_str());
	if (node == nullptr) {
		return false;
	}

	if (node->NumLeaves() > 1 && node->IsLeaf() == false) {
		std::stringstream ss;
		ss << "Command '" << lowercaseCommandName << "' resolved to " << node->NumLeaves()
		   << " possibilities:" << std::endl;

		ConsolePrint(ss.str().c_str());
		for (unsigned int i = 0; i < node->NumLeaves(); i++) {
			CommandTrie* leaf = node->GetLeaf(i);
			ConsolePrint(" ");
			ConsolePrint(leaf->GetValue());
		}
		ConsolePrint("\n");

		return false;
	}

	if (node->NumLeaves() == 0) {
		return false;
	}

	std::string resolvedCommandName = node->GetLeaf(0)->GetValue();

	// Search for the full command using the resolved name
	for (unsigned int i = 0; i < m_numConsoleCommands; i++) {
		if (resolvedCommandName == m_consoleCommands[i].command) {
			const char* params = nullptr;
			if (splitCommand.size() >= 2) {
				params = strstr(command, splitCommand[1].c_str());
			}

			((*this).*m_consoleCommands[i].func)(params);

			return true;
		}
	}

	return false;
}

unsigned int BaseApplication::NumPossibleCommands(const char* prefix) {
	CommandTrie* node = m_commandTrie->GetNode(prefix);

	if (node == nullptr) {
		return 0;
	}

	return node->NumLeaves();
}

const char* BaseApplication::GetPossibleCommand(const char* prefix, unsigned int index) {
	CommandTrie* node = m_commandTrie->GetNode(prefix);

	if (node == nullptr) {
		return 0;
	}

	if (index >= node->NumLeaves()) {
		return nullptr;
	}

	CommandTrie* leaf = node->GetLeaf(index);
	if (leaf == nullptr) {
		return nullptr;
	}

	return leaf->GetValue();
}

// Built-in console commands

void BaseApplication::CommandHelp(const char* /*params*/) {
	for (unsigned int i = 0; i < m_numConsoleCommands; i++) {
		std::string command = m_consoleCommands[i].command + std::string(" - ") + m_consoleCommands[i].helpText;
		ConsolePrint(command.c_str());
		ConsolePrint("\n");
	}

	ConsolePrint("\n");
}

void BaseApplication::CommandQuit(const char* /*params*/) {
	ConsolePrint("Shutting down...\n");
	RequestShutdown();
}

void BaseApplication::CommandLoad(const char* /*params*/) {
	char* fileSelected = nullptr;

	SelectFile(&fileSelected, nullptr);

	if (fileSelected != nullptr) {
		bool result = LoadRom(fileSelected);
		if (result == false) {
			ConsolePrint("Failed to load the specified file\n");
		}
		else {
			ConsolePrint("Loaded ");
			ConsolePrint(fileSelected);
			ConsolePrint("\n");
		}

		free(fileSelected);
	}
}

void BaseApplication::CommandPause(const char* /*params*/) {
	Pause();
	ConsolePrint("Emulation paused\n");
}
void BaseApplication::CommandRun(const char* /*params*/) {
	Run();
	ConsolePrint("Emulation resumed\n");
}

void BaseApplication::CommandSaveState(const char* params) {
	const char* stateName = "default";
	if (params != nullptr && strlen(params) != 0) {
		stateName = params;
	}

	SaveState(stateName);

	ConsolePrint("Saved state ");
	ConsolePrint(stateName);
	ConsolePrint("\n");
}

void BaseApplication::CommandLoadState(const char* params) {
	const char* stateName = "default";
	if (params != nullptr && strlen(params) != 0) {
		stateName = params;
	}

	LoadState(stateName);

	ConsolePrint("Loaded state ");
	ConsolePrint(stateName);
	ConsolePrint("\n");
}

void BaseApplication::CommandSpeed(const char* params) {
	double speed = 1.0;

	if (params != nullptr && strlen(params) > 0) {
		speed = atof(params);
	}

	SetEmulationSpeed((float)speed);

	std::stringstream ss;
	ss << "Set emulation speed to " << speed << std::endl;
	ConsolePrint(ss.str().c_str());
}

void BaseApplication::CommandMute(const char* /*params*/) {
	ConsolePrint("Unsupported\n");
}

void BaseApplication::CommandDisplayFilter(const char* in_params) {
	std::string params(in_params);

	if (params.empty()) {
		params = "none";
	}

	DisplayFilter::Type filter = DisplayFilter::NoFilter;

	if (params == "none" || params == "0" || params == "1") {
		filter = DisplayFilter::NoFilter;
	}
	else if (params == "hq2x" || params == "2x" || params == "2") {
		filter = DisplayFilter::Hq2x;
	}
	else if (params == "hq3x" || params == "3x" || params == "3") {
		filter = DisplayFilter::Hq3x;
	}
	else if (params == "hq4x" || params == "4x" || params == "4") {
		filter = DisplayFilter::Hq4x;
	}

	SetDisplayFilter(filter);

	const char* filterNames[] = {"None", "Hq2x", "Hq3x", "Hq4x"};
	ConsolePrint("Set display filter to ");
	ConsolePrint(filterNames[filter]);
	ConsolePrint("\n");
}

void BaseApplication::CommandVsync(const char* in_params) {
	std::string params(in_params);
	if (params.empty() || params == "0" || params == "off") {
		SetVsync(false);
		ConsolePrint("Vsync disabled\n");
	}
	else {
		SetVsync(true);
		ConsolePrint("Vsync enabled\n");
	}
}

void BaseApplication::CommandBackground(const char* in_params) {
	std::string params(in_params);
	if (params.empty() || params == "0" || params == "off") {
		DisableBackgroundAnimation();
		ConsolePrint("Disabled background animation\n");
	}
	else {
		EnableBackgroundAnimation();
		ConsolePrint("Enabled background animation\n");
	}
}
