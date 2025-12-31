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
#define EmulatedMachine_ToString 1

#include "emunisce.h"

#include <SDL3/SDL.h>

#include <fstream>
#include <string>

#include "console_debugger.h"
#include "input_manager.h"
#include "machine_feature.h"
#include "machine_includes.h"
#include "platform_includes.h"
#include "serialization/file_serializer.h"
#include "serialization/serialization_includes.h"

using namespace emunisce;

EmunisceApplication::EmunisceApplication() {
	// Create SDL window for GPU rendering
	m_window = SDL_CreateWindow("Emunisce", 320, 240, SDL_WINDOW_RESIZABLE);
	if (!m_window) {
		SDL_Log("Failed to create window: %s", SDL_GetError());
		return;
	}

	m_debugger = new ConsoleDebugger();
	m_renderer = new SDLGPURenderer();
	m_sound = new SDLSound();

	m_vsyncEnabled = true;

	m_fileDialogPending = false;
	m_fileDialogComplete = false;
	m_fileDialogResult = nullptr;
	m_fileDialogMask = nullptr;

	m_debugger->Initialize(this);
	m_renderer->Initialize(m_window);
	m_sound->Initialize(this);

	MapDefaultKeys();

	// Calling HandlePendingMachineChange here with just the MachineFeature components
	//(no emulated machine yet) so we don't have to force a LoadROM immediately.
	if (m_machine != nullptr) {
		m_pendingMachine = m_machine;
		HandlePendingMachineChange();
	}
}

EmunisceApplication::~EmunisceApplication() {
	if (m_sound) {
		m_sound->Shutdown();
		delete m_sound;
		m_sound = nullptr;
	}

	if (m_renderer) {
		m_renderer->Shutdown();
		delete m_renderer;
		m_renderer = nullptr;
	}

	if (m_debugger) {
		m_debugger->Shutdown();
		delete m_debugger;
		m_debugger = nullptr;
	}

	if (m_window) {
		SDL_DestroyWindow(m_window);
		m_window = nullptr;
	}
}

void EmunisceApplication::RunMainLoop() {
	while (!ShutdownRequested()) {
		HandleEvents();
		HandlePendingMachineChange();

		// Check for file dialog requests from other threads
		{
			std::lock_guard<std::mutex> lock(m_fileDialogMutex);
			if (m_fileDialogPending) {
				// Show dialog on main thread
				SDL_ShowOpenFileDialog(
					[](void* userdata, const char* const* filelist, int filter) {
						EmunisceApplication* app = (EmunisceApplication*)userdata;
						std::lock_guard<std::mutex> lock(app->m_fileDialogMutex);
						if (filelist && filelist[0]) {
							size_t len = strlen(filelist[0]) + 1;
							app->m_fileDialogResult = (char*)malloc(len);
							strcpy(app->m_fileDialogResult, filelist[0]);
						}
						app->m_fileDialogComplete = true;
						app->m_fileDialogCV.notify_one();
					},
					this, m_window, nullptr, 0, nullptr, false);

				m_fileDialogPending = false;
			}
		}

		EmulatedMachine* machine = GetMachine();
		if (machine) {
			EmulatedDisplay* display = machine->GetDisplay();
			if (display) {
				int frameCount = display->GetScreenBufferCount();
				int lastRendered = m_renderer->GetLastFrameRendered();

				// Debug logging
				static int logCounter = 0;
				if (logCounter++ % 120 == 0) {
					SDL_Log("Frame check: machine=%p, display=%p, lastRendered=%d, frameCount=%d", 
						(void*)machine, (void*)display, lastRendered, frameCount);
				}

				if (lastRendered != frameCount) {
					m_renderer->Draw(machine);
				}
				else {
					SDL_Delay(15);
				}
			}
			else {
				SDL_Log("Machine loaded but no display available");
				SDL_Delay(100);
			}
		}
			else {
			SDL_Delay(100);
		}
	}
}

SDL_Window* EmunisceApplication::GetWindow() {
	return m_window;
}

SDLGPURenderer* EmunisceApplication::GetRenderer() {
	return m_renderer;
}

SDLSound* EmunisceApplication::GetSound() {
	return m_sound;
}

ConsoleDebugger* EmunisceApplication::GetDebugger() {
	return m_debugger;
}

void EmunisceApplication::NotifyMachineChanged(EmulatedMachine* newMachine) {
	m_pendingMachine = newMachine;
	while (m_pendingMachine != nullptr) {
		SDL_Delay(10);
	}
}

void EmunisceApplication::RequestShutdown() {
	BaseApplication::RequestShutdown();
}

void EmunisceApplication::SetVsync(bool enabled) {
	m_vsyncEnabled = enabled;
	if (m_renderer) {
		m_renderer->SetVsync(enabled);
	}
}

void EmunisceApplication::DisplayStatusMessage(const char* message) {
	SDL_Log("Status: %s", message);
}

void EmunisceApplication::DisplayImportantMessage(MessageType::Type messageType, const char* message) {
	Uint32 flags = 0;
	if (messageType == MessageType::Information) {
		flags = SDL_MESSAGEBOX_INFORMATION;
	}
	else if (messageType == MessageType::Warning) {
		flags = SDL_MESSAGEBOX_WARNING;
	}
	else if (messageType == MessageType::Error) {
		flags = SDL_MESSAGEBOX_ERROR;
	}

	SDL_ShowSimpleMessageBox(flags, "Emunisce", message, m_window);
}

PromptResult::Type EmunisceApplication::DisplayPrompt(PromptType::Type promptType, const char* title,
													  const char* message, void** /*extraResult*/) {
	SDL_MessageBoxButtonData buttons[3];
	int numButtons = 0;

	if (promptType == PromptType::OkCancel) {
		buttons[0] = {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "OK"};
		buttons[1] = {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Cancel"};
		numButtons = 2;
	}
	else if (promptType == PromptType::YesNo) {
		buttons[0] = {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes"};
		buttons[1] = {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "No"};
		numButtons = 2;
	}
	else if (promptType == PromptType::YesNoCancel) {
		buttons[0] = {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Yes"};
		buttons[1] = {0, 1, "No"};
		buttons[2] = {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Cancel"};
		numButtons = 3;
	}

	SDL_MessageBoxData messageboxdata = {
		SDL_MESSAGEBOX_INFORMATION,
		m_window,
		title,
		message,
		numButtons,
		buttons,
		nullptr
	};

	int buttonid = -1;
	SDL_ShowMessageBox(&messageboxdata, &buttonid);

	if (promptType == PromptType::OkCancel) {
		return buttonid == 0 ? PromptResult::Ok : PromptResult::Cancel;
	}
	else if (promptType == PromptType::YesNo) {
		return buttonid == 0 ? PromptResult::Yes : PromptResult::No;
	}
	else if (promptType == PromptType::YesNoCancel) {
		if (buttonid == 0) return PromptResult::Yes;
		if (buttonid == 1) return PromptResult::No;
		return PromptResult::Cancel;
	}

	return PromptResult::Cancel;
}

bool EmunisceApplication::SelectFile(char** result, const char* fileMask) {
	if (result == nullptr) {
		return false;
	}

	SDL_Log("Opening file dialog...");

	// Request file dialog on main thread
	{
		std::unique_lock<std::mutex> lock(m_fileDialogMutex);
		m_fileDialogResult = nullptr;
		m_fileDialogMask = fileMask;
		m_fileDialogPending = true;
		m_fileDialogComplete = false;

		// Wait for dialog callback to complete
		m_fileDialogCV.wait(lock, [this]() {
			return m_fileDialogComplete;
		});
	}

	*result = m_fileDialogResult;

	if (*result != nullptr) {
		SDL_Log("File selected: %s", *result);
	}
	else {
		SDL_Log("File dialog cancelled or failed");
	}

	return *result != nullptr;
}

void EmunisceApplication::ConsolePrint(const char* text) {
	m_debugger->Print(text);
}

unsigned int EmunisceApplication::GetRomDataSize(const char* title) {
	std::string filename = GetCurrentRomDataFile(title);

	std::ifstream ifile;
	ifile.open(filename.c_str(), std::ios::in | std::ios::binary);

	if (!ifile.good()) {
		return 0;
	}

	ifile.seekg(0, std::ios::beg);
	unsigned int beginPosition = (int)ifile.tellg();

	ifile.seekg(0, std::ios::end);
	unsigned int endPosition = (int)ifile.tellg();

	return endPosition - beginPosition;
}

void EmunisceApplication::HandleEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_EVENT_QUIT:
				RequestShutdown();
				break;

			case SDL_EVENT_WINDOW_RESIZED:
				if (m_renderer) {
					m_renderer->Resize(event.window.data1, event.window.data2);
				}
				AdjustWindowSize();
				break;

			case SDL_EVENT_KEY_DOWN:
				m_inputManager->KeyDown(event.key.key);
				break;

			case SDL_EVENT_KEY_UP:
				m_inputManager->KeyUp(event.key.key);
				break;
		}
	}
}

void EmunisceApplication::HandlePendingMachineChange() {
	if (m_pendingMachine == nullptr) {
		return;
	}

	EmulatedMachine* newMachine = m_pendingMachine;

	// Call base class first to properly set m_machine
	BaseApplication::NotifyMachineChanged(newMachine);

	if (m_debugger) {
		m_debugger->SetMachine(m_machine);
	}

	if (m_renderer) {
		m_renderer->SetMachine(m_machine);
	}

	if (m_sound) {
		m_sound->SetMachine(m_machine);
	}

	// Start out at the native resolution or 320x240 (adjusted for aspect ratio), whichever is larger.
	ScreenResolution resolution = m_machine->GetScreenResolution();

	int targetWidth = 320;
	int targetHeight = 240;

	if (resolution.width >= 320) {
		targetWidth = resolution.width;
		targetHeight = resolution.height;
	}

	SDL_SetWindowSize(m_window, targetWidth, targetHeight);

	// Adjust to make up for aspect ratio
	AdjustWindowSize();

	m_pendingMachine = nullptr;
}

void EmunisceApplication::AdjustWindowSize() {
	if (m_machine == nullptr || m_window == nullptr) {
		return;
	}

	int windowWidth, windowHeight;
	SDL_GetWindowSize(m_window, &windowWidth, &windowHeight);

	ScreenResolution resolution = m_machine->GetDisplay()->GetScreenResolution();
	int nativeWidth = resolution.width;
	int nativeHeight = resolution.height;

	if ((windowWidth % nativeWidth) != 0 || (windowHeight % nativeHeight) != 0) {
		int xScale = windowWidth / nativeWidth;
		int yScale = windowHeight / nativeHeight;

		if (xScale == 0) xScale = 1;
		if (yScale == 0) yScale = 1;

		int scale = (xScale < yScale) ? xScale : yScale;

		int newWidth = nativeWidth * scale;
		int newHeight = nativeHeight * scale;

		SDL_SetWindowSize(m_window, newWidth, newHeight);
	}
}

Archive* EmunisceApplication::OpenFileArchive(const char* fileName, bool saving) {
	FileSerializer* serializer = new FileSerializer();
	serializer->SetFile(fileName);

	ArchiveMode::Type mode;
	if (saving == true) {
		mode = ArchiveMode::Saving;
	}
	else {
		mode = ArchiveMode::Loading;
	}

	Archive* archive = new Archive(serializer, mode);
	return archive;
}

void EmunisceApplication::ReleaseArchive(Archive* archive) {
	if (archive == nullptr) {
		return;
	}

	Serializer* serializer = archive->GetSerializer();

	delete archive;
	delete serializer;
}

Archive* EmunisceApplication::OpenRomData(const char* name, bool saving) {
	return OpenFileArchive(GetCurrentRomDataFile(name).c_str(), saving);
}

void EmunisceApplication::CloseRomData(Archive* archive) {
	ReleaseArchive(archive);
}

Archive* EmunisceApplication::OpenSavestate(const char* name, bool saving) {
	return OpenFileArchive(GetCurrentSaveStateFile(name).c_str(), saving);
}

void EmunisceApplication::CloseSavestate(Archive* archive) {
	ReleaseArchive(archive);
}

Archive* EmunisceApplication::OpenMovie(const char* name, bool saving) {
	return OpenFileArchive(GetCurrentMovieFile(name).c_str(), saving);
}

void EmunisceApplication::CloseMovie(Archive* archive) {
	ReleaseArchive(archive);
}

Archive* EmunisceApplication::OpenMacro(const char* name, bool saving) {
	return OpenFileArchive(GetCurrentMacroFile(name).c_str(), saving);
}

void EmunisceApplication::CloseMacro(Archive* archive) {
	ReleaseArchive(archive);
}

std::string EmunisceApplication::GetDataFolder() {
	const char* prefPath = SDL_GetPrefPath("Emunisce", "Emunisce");
	std::string result = prefPath ? prefPath : "./";
	return result;
}

std::string EmunisceApplication::GetRomDataFolder() {
	std::string path = GetDataFolder() + "RomData/";
	if (m_machine) {
		path += std::string(Machine::ToString[m_machine->GetType()]) + "/";
		path += std::string(m_machine->GetRomTitle()) + "/";
	}
	return path;
}

std::string EmunisceApplication::GetSavestateFolder() {
	std::string path = GetDataFolder() + "SaveStates/";
	if (m_machine) {
		path += std::string(Machine::ToString[m_machine->GetType()]) + "/";
		path += std::string(m_machine->GetRomTitle()) + "/";
	}
	return path;
}

std::string EmunisceApplication::GetMovieFolder() {
	std::string path = GetDataFolder() + "Movies/";
	if (m_machine) {
		path += std::string(Machine::ToString[m_machine->GetType()]) + "/";
		path += std::string(m_machine->GetRomTitle()) + "/";
	}
	return path;
}

std::string EmunisceApplication::GetMacroFolder() {
	return GetDataFolder() + "Macros/";
}

std::string EmunisceApplication::GetCurrentRomDataFile(const char* name) {
	return GetRomDataFolder() + std::string(name) + ".erd";
}

std::string EmunisceApplication::GetCurrentSaveStateFile(const char* name) {
	return GetSavestateFolder() + std::string(name) + ".ess";
}

std::string EmunisceApplication::GetCurrentMovieFile(const char* name) {
	return GetMovieFolder() + std::string(name) + ".eim";
}

std::string EmunisceApplication::GetCurrentMacroFile(const char* name) {
	return GetMacroFolder() + std::string(name) + ".eir";
}

void EmunisceApplication::MapDefaultKeys() {
	m_inputManager->MapKey("Up", SDLK_UP);
	m_inputManager->MapKey("Down", SDLK_DOWN);
	m_inputManager->MapKey("Left", SDLK_LEFT);
	m_inputManager->MapKey("Right", SDLK_RIGHT);

	m_inputManager->MapKey("B", SDLK_Q);
	m_inputManager->MapKey("B", SDLK_A);
	m_inputManager->MapKey("B", SDLK_Z);

	m_inputManager->MapKey("A", SDLK_W);
	m_inputManager->MapKey("A", SDLK_S);
	m_inputManager->MapKey("A", SDLK_X);

	m_inputManager->MapKey("Select", SDLK_V);
	m_inputManager->MapKey("Start", SDLK_B);

	m_inputManager->MapKey("Select", SDLK_LSHIFT);
	m_inputManager->MapKey("Select", SDLK_RSHIFT);
	m_inputManager->MapKey("Start", SDLK_RETURN);

	m_inputManager->MapKey("Select", SDLK_LEFTBRACKET);
	m_inputManager->MapKey("Start", SDLK_RIGHTBRACKET);

	m_inputManager->MapKey("Rewind", SDLK_TAB);
}
