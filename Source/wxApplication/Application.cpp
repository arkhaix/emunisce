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
#include "Application.h"
using namespace Emunisce;

//wx
#include "wx/sizer.h"
#include "WindowMain.h"

//Platform
#include "PlatformIncludes.h"

//Machine
#include "MachineIncludes.h"

//BaseApplication
#include "BaseApplication/MachineFeature.h"


Application::Application()
{
	m_renderer = new OpenGLRenderer();
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

    wxMessageBox(wxString::FromAscii(message), wxT("Emunisce"), iconType | wxOK);
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
    return false;
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
}

void Application::KeyUp(int key)
{
}


// BaseApplication interface

Archive* Application::OpenRomData(const char* name, bool saving)
{
    return NULL;
}

void Application::CloseRomData(Archive* archive)
{
}


Archive* Application::OpenSavestate(const char* name, bool saving)
{
    return NULL;
}

void Application::CloseSavestate(Archive* archive)
{
}


Archive* Application::OpenMovie(const char* name, bool saving)
{
    return NULL;
}

void Application::CloseMovie(Archive* archive)
{
}


Archive* Application::OpenMacro(const char* name, bool saving)
{
    return NULL;
}

void Application::CloseMacro(Archive* archive)
{
}



// wxApp

IMPLEMENT_APP(Application)

bool Application::OnInit()
{
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
    m_frame = new wxFrame((wxFrame *)NULL, -1,  wxT("Hello GL World"), wxPoint(50,50), wxSize(400,200));

    int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

    m_windowMain = new WindowMain(m_frame, args);
    m_windowMain->SetApplication(this);

    sizer->Add(m_windowMain, 1, wxEXPAND);

    m_frame->SetSizer(sizer);
    m_frame->SetAutoLayout(true);

    m_frame->Show();


	m_renderer->Initialize(this, NULL);

	if(m_machine != NULL)
	{
		BaseApplication::NotifyMachineChanged(m_machine);
		m_renderer->SetMachine(m_machine);
	}

	Run();

    return true;
}
