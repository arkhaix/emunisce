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

#include "wx/wx.h"

#include "BaseApplication/BaseApplication.h"
#include "OpenGLRenderer/OpenGLRenderer.h"

#include <string>
using namespace std;


namespace Emunisce
{

class WindowMain;

class Application : public wxApp, public BaseApplication
{
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
	virtual PromptResult::Type DisplayPrompt(PromptType::Type promptType, const char* title, const char* message, void** extraResult);

	virtual bool SelectFile(char** result, const char* fileMask);

	virtual unsigned int GetRomDataSize(const char* title);


	// IWindowMessageListener

	virtual void Closed();

	virtual void Draw();

	virtual void Resize(int newWidth, int newHeight);

	virtual void KeyDown(int key);
	virtual void KeyUp(int key);


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

	string GetDataFolder();

	string GetSaveStateFile(const char* name);
	string GetRomDataFile(const char* name);
	string GetMovieFile(const char* name);
	string GetMacroFile(const char* name);


	// Application properties

	OpenGLRenderer* m_renderer;


	// wxApp

    virtual bool OnInit();

    wxFrame* m_frame;
    WindowMain* m_windowMain;
};

}   //namespace Emunisce

#endif // APPLICATION_H
