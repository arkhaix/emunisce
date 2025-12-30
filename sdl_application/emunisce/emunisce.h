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
#ifndef EMUNISCE_SDL_H
#define EMUNISCE_SDL_H

#include <SDL3/SDL.h>

#include <condition_variable>
#include <mutex>
#include <string>

#include "base_application.h"
#include "sdl_gpu_renderer.h"

namespace emunisce {

class ConsoleDebugger;

class EmunisceApplication : public BaseApplication {
public:
	EmunisceApplication();
	~EmunisceApplication();

	void RunMainLoop();  ///< Main loop. Blocks until shutdown.

	SDL_Window* GetWindow();
	SDL_Renderer* GetRenderer();

	ConsoleDebugger* GetDebugger();

	// BaseApplication overrides

	void NotifyMachineChanged(EmulatedMachine* newMachine) override;
	void RequestShutdown() override;

	// BaseApplication interface

	void SetVsync(bool enabled) override;

	void DisplayStatusMessage(const char* message) override;
	void DisplayImportantMessage(MessageType::Type messageType, const char* message) override;
	PromptResult::Type DisplayPrompt(PromptType::Type promptType, const char* title, const char* message,
											 void** extraResult) override;

	bool SelectFile(char** result, const char* fileMask) override;

	void ConsolePrint(const char* text) override;

	unsigned int GetRomDataSize(const char* title) override;

private:
	void HandleEvents();
	void HandlePendingMachineChange();
	void AdjustWindowSize();

	Archive* OpenFileArchive(const char* fileName, bool saving);
	void ReleaseArchive(Archive* archive);

	// BaseApplication interface

	Archive* OpenRomData(const char* name, bool saving) override;
	void CloseRomData(Archive* archive) override;

	Archive* OpenSavestate(const char* name, bool saving) override;
	void CloseSavestate(Archive* archive) override;

	Archive* OpenMovie(const char* name, bool saving) override;
	void CloseMovie(Archive* archive) override;

	Archive* OpenMacro(const char* name, bool saving) override;
	void CloseMacro(Archive* archive) override;

	virtual std::string GetDataFolder();
	virtual std::string GetRomDataFolder();
	virtual std::string GetSavestateFolder();
	virtual std::string GetMovieFolder();
	virtual std::string GetMacroFolder();

	std::string GetCurrentRomDataFile(const char* name);
	std::string GetCurrentSaveStateFile(const char* name);
	std::string GetCurrentMovieFile(const char* name);
	std::string GetCurrentMacroFile(const char* name);

	void MapDefaultKeys();

	SDL_Window* m_window;

	ConsoleDebugger* m_debugger;
	SDLGPURenderer* m_renderer;

	bool m_vsyncEnabled;

	EmulatedMachine* m_pendingMachine;

	// File dialog state for thread-safe handling
	std::mutex m_fileDialogMutex;
	std::condition_variable m_fileDialogCV;
	char* m_fileDialogResult;
	bool m_fileDialogPending;
	bool m_fileDialogComplete;
	const char* m_fileDialogMask;
};

}  // namespace emunisce

#endif
