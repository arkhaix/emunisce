/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "UserInterface.h"

#include "windows.h"


// User to application


//Rom

bool UserInterface::LoadRom(const char* filename)
{
	return false;
}

void UserInterface::Reset()
{
}


//Emulation

void UserInterface::SetEmulationSpeed(float speed)
{
}


void UserInterface::Run()
{
}

void UserInterface::Pause()
{
}


void UserInterface::StepInstruction()
{
}

void UserInterface::StepFrame()
{
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

	char selectedFile[MAX_PATH];

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
