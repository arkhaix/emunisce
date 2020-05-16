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
#ifndef EMUNISCE_H
#define EMUNISCE_H

//BaseApplication
#include "BaseApplication/BaseApplication.h"
#include "OpenGLRenderer/OpenGLRenderer.h"

//WindowsApplication
#include "../GdiPlusRenderer/GdiPlusRenderer.h"
#include "../WaveOutSound/WaveOutSound.h"

#include "../Win32Common/Window.h"

#include <string>


namespace Emunisce
{

class ConsoleDebugger;

class GdiPlusRenderer;
class WaveOutSound;


class EmunisceApplication : public BaseApplication, public IWindowMessageListener
{
public:

	EmunisceApplication();
	~EmunisceApplication();

	void RunWindow();	///<Pumps messages on the window until shutdown is requested.  Blocks until shutdown.

	Window* GetWindow();

	ConsoleDebugger* GetDebugger();
	WaveOutSound* GetSound();


	// BaseApplication overrides

	virtual void NotifyMachineChanged(IEmulatedMachine* newMachine);
	virtual void RequestShutdown();


	// BaseApplication interface

	virtual void SetVsync(bool enabled);

	virtual void DisplayStatusMessage(const char* message);
	virtual void DisplayImportantMessage(MessageType::Type messageType, const char* message);
	virtual PromptResult::Type DisplayPrompt(PromptType::Type promptType, const char* title, const char* message, void** extraResult);

	virtual bool SelectFile(char** result, const char* fileMask);

	void ConsolePrint(const char* text) override;

	virtual unsigned int GetRomDataSize(const char* title);


	// IWindowMessageListener

	virtual void Closed();

	virtual void Draw();

	virtual void Resize(int newWidth, int newHeight);

	virtual void KeyDown(int key);
	virtual void KeyUp(int key);


private:

	void AdjustWindowSize();
	void HandlePendingMachineChange();

	Archive* OpenFileArchive(const char* fileName, bool saving);
	void ReleaseArchive(Archive* archive);


	// BaseApplication interface

	virtual Archive* OpenRomData(const char* name, bool saving);
	virtual void CloseRomData(Archive* archive);

	virtual Archive* OpenSavestate(const char* name, bool saving);
	virtual void CloseSavestate(Archive* archive);

	virtual Archive* OpenMovie(const char* name, bool saving);
	virtual void CloseMovie(Archive* archive);

	virtual Archive* OpenMacro(const char* name, bool saving);
	virtual void CloseMacro(Archive* archive);


	// Persistence

	std::string GetDataFolder();

	std::string GetBaseSaveStateFolder();
	std::string GetCurrentSaveStateFolder();
	std::string GetCurrentSaveStateFile(const char* id);

	std::string GetBaseRomDataFolder();
	std::string GetCurrentRomDataFolder();
	std::string GetCurrentRomDataFile(const char* name);

	std::string GetBaseMovieFolder();
	std::string GetCurrentMovieFolder();
	std::string GetCurrentMovieFile(const char* name);

	std::string GetBaseMacroFolder();
	std::string GetCurrentMacroFolder();
	std::string GetCurrentMacroFile(const char* name);


	// Input

	void MapDefaultKeys();


	Window* m_window;

	IEmulatedMachine* m_pendingMachine;

	ConsoleDebugger* m_debugger;

	//GdiPlusRenderer* m_renderer;
	OpenGLRenderer* m_renderer;
	WaveOutSound* m_sound;
};

}	//namespace Emunisce

#endif
