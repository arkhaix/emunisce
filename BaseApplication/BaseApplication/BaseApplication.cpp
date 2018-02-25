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
#include "BaseApplication.h"
using namespace Emunisce;

#include <string>
#include <vector>
using namespace std;

#include "PlatformIncludes.h"

#include "MachineIncludes.h"

#include "Serialization/SerializationIncludes.h"
#include "Serialization/MemorySerializer.h"

#include "MachineFeature.h"
#include "Gui.h"
#include "Rewinder.h"
#include "InputRecording.h"

#include "InputManager.h"
#include "MachineRunner.h"


// BaseApplication

BaseApplication::BaseApplication()
{
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
}

BaseApplication::~BaseApplication()
{
	m_shutdownRequested = true;

	m_machineRunner->Shutdown();
	delete m_machineRunner;

	//input manager doesn't need shutdown
    delete m_inputManager;

	delete m_inputRecorder;
	delete m_rewinder;
	delete m_gui;

	if(m_wrappedMachine)
	{
		MachineFactory::ReleaseMachine(m_wrappedMachine);
	}
}


//Emulated machine
void BaseApplication::NotifyMachineChanged(IEmulatedMachine* newMachine)
{
	if(newMachine != m_gui && newMachine != m_inputRecorder && newMachine != m_rewinder)
	{
		m_gui->SetFocus(false);
		m_machine->SetEmulatedMachine(newMachine);
		m_wrappedMachine = newMachine;
	}

    m_inputManager->SetMachine(m_machine);
	m_machineRunner->SetMachine(m_machine);

	newMachine->SetApplicationInterface(this);
}

IEmulatedMachine* BaseApplication::GetMachine()
{
	return m_machine;
}


//Machine features
Gui* BaseApplication::GetGui()
{
	return m_gui;
}

Rewinder* BaseApplication::GetRewinder()
{
	return m_rewinder;
}

InputRecording* BaseApplication::GetInputRecorder()
{
	return m_inputRecorder;
}


InputManager* BaseApplication::GetInputManager()
{
    return m_inputManager;
}

MachineRunner* BaseApplication::GetMachineRunner()
{
	return m_machineRunner;
}


//Shutdown
bool BaseApplication::ShutdownRequested()
{
	return m_shutdownRequested;
}

void BaseApplication::RequestShutdown()
{
	m_shutdownRequested = true;
}



// IUserInterface

//User to application

//Rom
bool BaseApplication::LoadRom(const char* filename)
{
	//Prompt for a file if one wasn't provided
	char* selectedFilename = nullptr;
	if(filename == nullptr || strlen(filename) == 0)
	{
		bool fileSelected = SelectFile(&selectedFilename);

		if(fileSelected == false)
			return false;

		filename = selectedFilename;
	}

	//Load it
	IEmulatedMachine* machine = MachineFactory::CreateMachine(filename);
	if(machine == nullptr)
	{
		if(selectedFilename != nullptr)
			free((void*)selectedFilename);

		return false;
	}

	//Successfully created a new Machine
	IEmulatedMachine* oldMachine = m_wrappedMachine;

	bool wasPaused = m_machineRunner->IsPaused();
	m_machineRunner->Pause();

	//Let everyone know that the old one is going away
	NotifyMachineChanged(machine);

	if(wasPaused == false)
		m_machineRunner->Run();

	//Release the old one
	if(oldMachine != nullptr)
		MachineFactory::ReleaseMachine(oldMachine);

	//Remember the file used so we can reset later if necessary
	strcpy_s(m_lastRomLoaded, 1024, filename);

	if(selectedFilename != nullptr)
		free((void*)selectedFilename);

	return true;
}

void BaseApplication::Reset()
{
	LoadRom(m_lastRomLoaded);
}


//Emulation
void BaseApplication::SetEmulationSpeed(float multiplier)
{
	m_machineRunner->SetEmulationSpeed(multiplier);
}


void BaseApplication::Run()
{
	m_machineRunner->Run();
}

void BaseApplication::Pause()
{
	m_machineRunner->Pause();
}


void BaseApplication::StepInstruction()
{
	m_machineRunner->StepInstruction();
}

void BaseApplication::StepFrame()
{
	m_machineRunner->StepFrame();
}


//State
void BaseApplication::SaveState(const char* name)
{
	Archive* archive = OpenSavestate(name, true);
	if(archive == nullptr)
		return;

	bool wasPaused = m_machineRunner->IsPaused();
	m_machineRunner->Pause();

	m_machine->SaveState(*archive);

	if(wasPaused == false)
		m_machineRunner->Run();

	CloseSavestate(archive);
}

void BaseApplication::LoadState(const char* name)
{
	Archive* archive = OpenSavestate(name, false);
	if(archive == nullptr)
		return;

	bool wasPaused = m_machineRunner->IsPaused();
	m_machineRunner->Pause();

	m_machine->LoadState(*archive);

	if(wasPaused == false)
		m_machineRunner->Run();

	CloseSavestate(archive);
}


//Gui
void BaseApplication::EnableBackgroundAnimation()
{
	if(m_gui != nullptr)
		m_gui->EnableBackgroundAnimation();
}

void BaseApplication::DisableBackgroundAnimation()
{
	if(m_gui != nullptr)
		m_gui->DisableBackgroundAnimation();
}


//Display
void BaseApplication::SetDisplayFilter(DisplayFilter::Type displayFilter)
{
	if(m_gui != nullptr)
		m_gui->SetDisplayFilter(displayFilter);
}


//Input movie
void BaseApplication::StartRecordingInput()
{
	if(m_inputRecorder != nullptr)
		m_inputRecorder->StartRecording();
}

void BaseApplication::StopRecordingInput()
{
	if(m_inputRecorder != nullptr)
		m_inputRecorder->StopRecording();
}


void BaseApplication::PlayMovie()
{
	if(m_inputRecorder != nullptr)
		m_inputRecorder->StartPlayback(true, true, false);
}

void BaseApplication::StopMovie()
{
	if(m_inputRecorder != nullptr)
		m_inputRecorder->StopPlayback();
}


void BaseApplication::SaveMovie(const char* name)
{
	Archive* archive = OpenMovie(name, true);
	if(archive == nullptr)
		return;

	m_inputRecorder->SerializeMovie(*archive);

	CloseMovie(archive);
}

void BaseApplication::LoadMovie(const char* name)
{
	Archive* archive = OpenMovie(name, false);
	if(archive == nullptr)
		return;

	m_inputRecorder->SerializeMovie(*archive);

	CloseMovie(archive);
}


void BaseApplication::PlayMacro(bool loop)
{
	if(m_inputRecorder != nullptr)
		m_inputRecorder->StartPlayback(false, false, loop);
}

void BaseApplication::StopMacro()
{
	if(m_inputRecorder != nullptr)
		m_inputRecorder->StopPlayback();
}


void BaseApplication::SaveMacro(const char* name)
{
	Archive* archive = OpenMacro(name, true);
	if(archive == nullptr)
		return;

	m_inputRecorder->SerializeHistory(*archive);

	CloseMacro(archive);
}

void BaseApplication::LoadMacro(const char* name)
{
	Archive* archive = OpenMacro(name, false);
	if(archive == nullptr)
		return;

	m_inputRecorder->SerializeHistory(*archive);

	CloseMacro(archive);
}


// IMachineToApplication

void BaseApplication::HandleApplicationEvent(unsigned int eventId)
{
	if(eventId >= 0x01000000 && eventId < 0x02000000)
	{
		if(m_inputRecorder != nullptr)
			m_inputRecorder->ApplicationEvent(eventId);
	}
	else if(eventId >= 0x02000000 && eventId < 0x03000000)
	{
		if(m_rewinder != nullptr)
			m_rewinder->ApplicationEvent(eventId);
	}
}


void BaseApplication::SaveRomData(const char* title, unsigned char* buffer, unsigned int bytes)
{
	Archive* archive = OpenRomData(title, true);
	if(archive == nullptr)
		return;

	archive->SerializeBuffer(buffer, bytes);

	CloseRomData(archive);
}

void BaseApplication::LoadRomData(const char* title, unsigned char* buffer, unsigned int bytes)
{
	Archive* archive = OpenRomData(title, false);
	if(archive == nullptr)
		return;

	archive->SerializeBuffer(buffer, bytes);

	CloseRomData(archive);
}


void BaseApplication::AddConsoleCommandHandler(const char* command, ConsoleCommandHandler func)
{
    if(m_numConsoleCommands >= MaxConsoleCommands)
        return;

    if(func == nullptr)
        return;
    
    strcpy_s(m_consoleCommands[m_numConsoleCommands].command, 16, command);
    m_consoleCommands[m_numConsoleCommands].func = func;

    m_numConsoleCommands++;
}
    
unsigned int BaseApplication::NumConsoleCommands()
{
   return m_numConsoleCommands; 
}

const char* BaseApplication::GetConsoleCommand(unsigned int index)
{
    if(index >= m_numConsoleCommands)
        return nullptr;
    
    return m_consoleCommands[index].command;
}

vector<string> SplitCommand(string command)
{
	vector<string> result;

	char* input = const_cast<char*>(command.c_str());
	const char* separators = " \t\n";
	char* token = nullptr;
	char* context = nullptr;

	token = strtok_s(input, separators, &context);
	while(token != nullptr)
	{
		result.push_back(string(token));
		token = strtok_s(nullptr, separators, &context);
	}

	return result;
}

void BaseApplication::ExecuteConsoleCommand(const char* command)
{
    vector<string> splitCommand = SplitCommand(command);
    if(splitCommand.size() < 1)
        return;

    const char* commandName = splitCommand[0].c_str();

    for(unsigned int i = 0; i < m_numConsoleCommands; i++)
    {
        if(strcmp(commandName, m_consoleCommands[i].command) == 0)
        {
            const char* params = nullptr;
            if(splitCommand.size() >= 2)
                params = strstr(command, splitCommand[1].c_str());

            ((*this).*m_consoleCommands[i].func)(commandName, params);
        }
    }
}
