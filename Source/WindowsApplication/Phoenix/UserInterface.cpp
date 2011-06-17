/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

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
#include "UserInterface.h"
using namespace Emunisce;

#include "windows.h"

#include "Phoenix.h"
#include "MachineRunner.h"


// Application component

UserInterface::UserInterface()
{
	m_phoenix = NULL;
	m_runner = NULL;
}

UserInterface::~UserInterface()
{
}


void UserInterface::Initialize(Phoenix* phoenix)
{
	m_phoenix = phoenix;
	m_runner = phoenix->GetMachineRunner();
}

void UserInterface::Shutdown()
{
}

void UserInterface::SetMachine(IEmulatedMachine* machine)
{
}



// User to application


//Rom

bool UserInterface::LoadRom(const char* filename)
{
	return m_phoenix->LoadRom(filename);
}

void UserInterface::Reset()
{
	m_phoenix->ResetRom();
}


//Emulation

void UserInterface::SetEmulationSpeed(float multiplier)
{
	m_runner->SetEmulationSpeed(multiplier);
}


void UserInterface::Run()
{
	m_runner->Run();
}

void UserInterface::Pause()
{
	m_runner->Pause();
}


void UserInterface::StepInstruction()
{
	m_runner->StepInstruction();
}

void UserInterface::StepFrame()
{
	m_runner->StepFrame();
}



// Application to user

void UserInterface::DisplayStatusMessage(const char* message)
{
}


void UserInterface::DisplayImportantMessage(MessageType::Type messageType, const char* message)
{
	int iconType = 0;
	if(messageType == MessageType::Information)
		iconType = MB_ICONINFORMATION;
	else if(messageType == MessageType::Warning)
		iconType = MB_ICONWARNING;
	else if(messageType == MessageType::Error)
		iconType = MB_ICONERROR;

	MessageBox(NULL, message, "Phoenix", iconType | MB_OK);
}


PromptResult::Type UserInterface::DisplayPrompt(PromptType::Type promptType, const char* title, const char* message, void** extraResult)
{
	int windowsPromptType = MB_OK;
	if(promptType == PromptType::OkCancel)
		windowsPromptType = MB_OKCANCEL;
	else if(promptType == PromptType::YesNo)
		windowsPromptType = MB_YESNO;
	else if(promptType == PromptType::YesNoCancel)
		windowsPromptType = MB_YESNOCANCEL;

	int windowsResult = MessageBox(NULL, message, title, windowsPromptType);

	PromptResult::Type result = PromptResult::Cancel;
	if(result == IDOK)
		result = PromptResult::Ok;
	else if(result == IDCANCEL)
		result = PromptResult::Cancel;
	else if(result == IDYES)
		result = PromptResult::Yes;
	else if(result == IDNO)
		result = PromptResult::No;

	return result;
}


bool UserInterface::SelectFile(char** result, const char* fileMask)
{
	if(result == NULL)
		return false;

	char selectedFile[MAX_PATH] = {0};

	OPENFILENAME openDialog;
    ZeroMemory(&openDialog, sizeof(openDialog));

    openDialog.lStructSize = sizeof(openDialog);
    openDialog.lpstrFilter = fileMask;
    openDialog.lpstrFile = selectedFile;
    openDialog.nMaxFile = MAX_PATH;
    openDialog.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&openDialog))
	{
		*result = (char*)malloc(MAX_PATH);
		strcpy_s(*result, MAX_PATH, selectedFile);
		return true;
	}

	return false;
}
