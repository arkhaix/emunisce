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

#include "Application.h"
using namespace Emunisce;

//wx
#include "wx/sizer.h"
#include "WindowMain.h"

//Platform
#include "PlatformIncludes.h"

#ifdef EMUNISCE_PLATFORM_LINUX
#include <sys/types.h>
#include <sys/stat.h>
#endif

//Machine
#include "MachineIncludes.h"

//BaseApplication
#include "BaseApplication/InputManager.h"
#include "BaseApplication/MachineFeature.h"

//Serialization
#include "Serialization/SerializationIncludes.h"
#include "Serialization/FileSerializer.h"


Application::Application()
{
	m_renderer = new OpenGLRenderer();

	MapDefaultKeys();
}

Application::~Application()
{
}



// BaseApplication overrides

void Application::NotifyMachineChanged(IEmulatedMachine* newMachine)
{
	BaseApplication::NotifyMachineChanged(newMachine);
}

void Application::RequestShutdown()
{
	BaseApplication::RequestShutdown();
	m_windowMain->Close();
	m_frame->Close();
}



// BaseApplication interface

void Application::SetVsync(bool enabled)
{
	if(m_renderer != NULL)
		m_renderer->SetVsync(enabled);
}


void Application::DisplayStatusMessage(const char* message)
{
}

void Application::DisplayImportantMessage(MessageType::Type messageType, const char* message)
{
    long iconType = wxICON_INFORMATION;

	if(messageType == MessageType::Information)
		iconType = wxICON_INFORMATION;
	else if(messageType == MessageType::Warning)
		iconType = wxICON_WARNING;
	else if(messageType == MessageType::Error)
		iconType = wxICON_ERROR;

    wxMessageBox(wxString::FromAscii(message), _("Emunisce"), iconType | wxOK);
}

PromptResult::Type Application::DisplayPrompt(PromptType::Type promptType, const char* title, const char* message, void** extraResult)
{
    long wxPromptType = wxOK;
	if(promptType == PromptType::OkCancel)
		wxPromptType = wxOK | wxCANCEL;
	else if(promptType == PromptType::YesNo)
		wxPromptType = wxYES | wxNO;
	else if(promptType == PromptType::YesNoCancel)
		wxPromptType = wxYES | wxNO | wxCANCEL;

	int wxResult = wxMessageBox( wxString::FromAscii(message), wxString::FromAscii(title), wxPromptType);

	PromptResult::Type result = PromptResult::Cancel;
	if(wxResult == wxOK)
		result = PromptResult::Ok;
	else if(wxResult == wxCANCEL)
		result = PromptResult::Cancel;
	else if(wxResult == wxYES)
		result = PromptResult::Yes;
	else if(wxResult == wxNO)
		result = PromptResult::No;

	return result;
}


bool Application::SelectFile(char** result, const char* fileMask)
{
    if(result == NULL)
		return false;

    if(fileMask == NULL)
        fileMask = "*.*|*.*";

    wxFileDialog openFileDialog(NULL, _("Open File"), _(""), _(""), wxString::FromAscii(fileMask), wxFD_OPEN|wxFD_FILE_MUST_EXIST);

    if (openFileDialog.ShowModal() == wxID_CANCEL)
        return false;

    int bufferSize = openFileDialog.GetPath().Length()+1;

    *result = (char*)malloc(bufferSize);
    strcpy_s(*result, bufferSize, openFileDialog.GetPath().ToAscii());

    return true;
}


unsigned int Application::GetRomDataSize(const char* title)
{
    return 0;
}



// IWindowMessageListener

void Application::Closed()
{
}


void Application::Draw()
{
	if(m_renderer != NULL)
		m_renderer->Draw();
}


void Application::Resize(int newWidth, int newHeight)
{
	if(m_renderer != NULL)
		m_renderer->Resize(newWidth, newHeight);
}


void Application::KeyDown(int key)
{
    char* fileSelected = NULL;

    if(key >= 'a' && key <= 'z')
        key = 'A' + (key-'a');

    if(key == 'Q' || key == WXK_ESCAPE)
        RequestShutdown();

    else if(key == 'T')
        DisplayImportantMessage(MessageType::Information, "test");

    else if(key == 'O')
    {
        SelectFile(&fileSelected, NULL);

        if(fileSelected != NULL)
        {
            bool result = LoadRom(fileSelected);
            if(result == false)
                DisplayImportantMessage(MessageType::Error, "Failed to load the specified file.");

            free(fileSelected);
        }
    }

    else
    {
        m_inputManager->KeyDown(key);
    }
}

void Application::KeyUp(int key)
{
    if(key >= 'a' && key <= 'z')
        key = 'A' + (key-'a');

    m_inputManager->KeyUp(key);
}


// BaseApplication interface

Archive* Application::OpenRomData(const char* name, bool saving)
{
    return OpenFileArchive( GetRomDataFile(name).c_str(), saving );
}

void Application::CloseRomData(Archive* archive)
{
    ReleaseArchive(archive);
}


Archive* Application::OpenSavestate(const char* name, bool saving)
{
    return OpenFileArchive( GetSaveStateFile(name).c_str(), saving );
}

void Application::CloseSavestate(Archive* archive)
{
    ReleaseArchive(archive);
}


Archive* Application::OpenMovie(const char* name, bool saving)
{
    return OpenFileArchive( GetMovieFile(name).c_str(), saving );
}

void Application::CloseMovie(Archive* archive)
{
    ReleaseArchive(archive);
}


Archive* Application::OpenMacro(const char* name, bool saving)
{
    return OpenFileArchive( GetMacroFile(name).c_str(), saving );
}

void Application::CloseMacro(Archive* archive)
{
    ReleaseArchive(archive);
}



//  Persistence

Archive* Application::OpenFileArchive(const char* filename, bool saving)
{
    FileSerializer* serializer = new FileSerializer();
	serializer->SetFile(filename);

	ArchiveMode::Type mode;
	if(saving == true)
		mode = ArchiveMode::Saving;
	else
		mode = ArchiveMode::Loading;

	Archive* archive = new Archive(serializer, mode);
	return archive;
}

void Application::ReleaseArchive(Archive* archive)
{
    if(archive == NULL)
		return;

	ISerializer* serializer = archive->GetSerializer();

	archive->Close();

	delete archive;
	delete serializer;
}


string Application::GetDataFolder()
{
#ifdef EMUNISCE_PLATFORM_WINDOWS
    //todo
    return string(".");

#elif EMUNISCE_PLATFORM_LINUX
    string result = getenv("HOME");
    if(result.length() == 0)
        result = ".";

    result += string("/.Emunisce");

    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    return result;

#endif
}


string Application::GetSaveStateFile(const char* name)
{
#ifdef EMUNISCE_PLATFORM_WINDOWS
    //todo
    return GetDataFolder() + string("/") + string(name) + string(".ess");

#elif EMUNISCE_PLATFORM_LINUX
    string result = GetDataFolder();

    result += string("/SaveStates");
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(EmulatedMachine::ToString[ m_machine->GetType() ]);
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(m_machine->GetRomTitle());
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(name) + string(".ess");

    return result;
#endif
}

string Application::GetRomDataFile(const char* name)
{
#ifdef EMUNISCE_PLATFORM_WINDOWS
    //todo
    return GetDataFolder() + string("/") + string(name) + string(".erd");

#elif EMUNISCE_PLATFORM_LINUX
    string result = GetDataFolder();

    result += string("/RomData");
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(EmulatedMachine::ToString[ m_machine->GetType() ]);
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(m_machine->GetRomTitle());
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(name) + string(".erd");

    return result;
#endif
}

string Application::GetMovieFile(const char* name)
{
#ifdef EMUNISCE_PLATFORM_WINDOWS
    //todo
    return GetDataFolder() + string("/") + string(name) + string(".eim");

#elif EMUNISCE_PLATFORM_LINUX
    string result = GetDataFolder();

    result += string("/Movies");
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(EmulatedMachine::ToString[ m_machine->GetType() ]);
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(m_machine->GetRomTitle());
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(name) + string(".eim");

    return result;
#endif
}

string Application::GetMacroFile(const char* name)
{
#ifdef EMUNISCE_PLATFORM_WINDOWS
    //todo
    return GetDataFolder() + string("/") + string(name) + string(".eir");

#elif EMUNISCE_PLATFORM_LINUX
    string result = GetDataFolder();

    result += string("/Macros");
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    result += string("/") + string(EmulatedMachine::ToString[ m_machine->GetType() ]);
    mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    //no rom title folder for macros.  they're global to the machine.

    result += string("/") + string(name) + string(".eir");

    return result;
#endif
}



// Input

void Application::MapDefaultKeys()
{
    m_inputManager->MapKey("Up", WXK_UP);
    m_inputManager->MapKey("Down", WXK_DOWN);
    m_inputManager->MapKey("Left", WXK_LEFT);
    m_inputManager->MapKey("Right", WXK_RIGHT);

    m_inputManager->MapKey("B", 'Q');
    m_inputManager->MapKey("B", 'A');
    m_inputManager->MapKey("B", 'Z');

    m_inputManager->MapKey("A", 'W');
    m_inputManager->MapKey("A", 'S');
    m_inputManager->MapKey("A", 'X');

    m_inputManager->MapKey("Select", 'V');
    m_inputManager->MapKey("Start", 'B');

    m_inputManager->MapKey("Select", WXK_SHIFT);
    m_inputManager->MapKey("Start", WXK_RETURN);

    m_inputManager->MapKey("Select", '[');
    m_inputManager->MapKey("Start", ']');

    m_inputManager->MapKey("Rewind", WXK_TAB);
}



// wxApp

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_frame = new wxFrame((wxFrame *)NULL, -1, wxT("Emunisce"), wxPoint(50,50), wxSize(320,240));

    int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

    m_windowMain = new WindowMain(m_frame, args);
    m_windowMain->SetApplication(this);

    sizer->Add(m_windowMain, 1, wxEXPAND);

    m_frame->SetSizer(sizer);
    m_frame->SetAutoLayout(true);

    m_frame->Show();

    m_windowMain->SetFocus();


	m_renderer->Initialize(this, NULL);

	if(m_machine != NULL)
	{
		BaseApplication::NotifyMachineChanged(m_machine);
		m_renderer->SetMachine(m_machine);
	}

	Run();

    return true;
}
