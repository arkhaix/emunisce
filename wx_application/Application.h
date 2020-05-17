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
#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>

#include "BaseApplication/BaseApplication.h"
#include "OpenGLRenderer/OpenGLRenderer.h"
#include "wx/wx.h"

namespace Emunisce {

class WindowMain;
class ConsoleWindow;

class Application : public wxApp, public BaseApplication {
public:
	Application();
	~Application();

	// BaseApplication overrides

	virtual void NotifyMachineChanged(IEmulatedMachine* newMachine);
	virtual void RequestShutdown();

	// BaseApplication interface

	virtual void SetVsync(bool enabled);

	virtual void DisplayStatusMessage(const char* message);
	virtual void DisplayImportantMessage(MessageType::Type messageType, const char* message);
	virtual PromptResult::Type DisplayPrompt(PromptType::Type promptType, const char* title, const char* message,
											 void** extraResult);

	virtual bool SelectFile(char** result, const char* fileMask);

	virtual void ConsolePrint(const char* text);

	virtual unsigned int GetRomDataSize(const char* title);

	// IWindowMessageListener

	virtual void Closed();

	virtual void Draw();

	virtual void Resize(int newWidth, int newHeight);

	virtual void KeyDown(int key);
	virtual void KeyUp(int key);

	// wxApplication

	void ShowConsoleWindow();
	void ShowGameWindow();

	virtual bool ExecuteConsoleCommand(const char* command);  ///< Move to public for use from ConsoleWindow

protected:
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

	Archive* OpenFileArchive(const char* filename, bool saving);
	void ReleaseArchive(Archive* archive);

	std::string GetDataFolder();

	std::string GetSaveStateFile(const char* name);
	std::string GetRomDataFile(const char* name);
	std::string GetMovieFile(const char* name);
	std::string GetMacroFile(const char* name);

	// Input

	void MapDefaultKeys();

	// Application properties

	OpenGLRenderer* m_renderer;

	// wxApp

	virtual bool OnInit();

	wxFrame* m_frame;
	WindowMain* m_windowMain;
	ConsoleWindow* m_consoleWindow;
};

}  // namespace Emunisce

#endif  // APPLICATION_H
