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
using namespace emunisce;

// wx
#include "ConsoleWindow.h"
#include "WindowMain.h"
#include "wx/sizer.h"
#include "wx/textctrl.h"

// Platform
#include "platform_includes.h"

#ifdef EMUNISCE_PLATFORM_LINUX
#include <sys/stat.h>
#include <sys/types.h>
#endif

// Machine
#include "machine_includes.h"

// BaseApplication
#include "base_application/input_manager.h"
#include "base_application/machine_feature.h"

// Serialization
#include "serialization/file_serializer.h"
#include "serialization/serialization_includes.h"

Application::Application() {
	m_renderer = new OpenGLRenderer();

	m_consoleWindow = nullptr;

	MapDefaultKeys();
}

Application::~Application() {
}

// BaseApplication overrides

void Application::NotifyMachineChanged(EmulatedMachine* newMachine) {
	BaseApplication::NotifyMachineChanged(newMachine);
}

void Application::RequestShutdown() {
	BaseApplication::RequestShutdown();
	m_consoleWindow->Close();
	m_windowMain->Close();
	m_frame->Close();
}

// BaseApplication interface

void Application::SetVsync(bool enabled) {
	if (m_renderer != nullptr)
		m_renderer->SetVsync(enabled);
}

void Application::DisplayStatusMessage(const char* message) {
}

void Application::DisplayImportantMessage(MessageType::Type messageType, const char* message) {
	long iconType = wxICON_INFORMATION;

	if (messageType == MessageType::Information)
		iconType = wxICON_INFORMATION;
	else if (messageType == MessageType::Warning)
		iconType = wxICON_WARNING;
	else if (messageType == MessageType::Error)
		iconType = wxICON_ERROR;

	wxMessageBox(wxString::FromAscii(message), _("Emunisce"), iconType | wxOK);
}

PromptResult::Type Application::DisplayPrompt(PromptType::Type promptType, const char* title, const char* message,
											  void** extraResult) {
	long wxPromptType = wxOK;
	if (promptType == PromptType::OkCancel)
		wxPromptType = wxOK | wxCANCEL;
	else if (promptType == PromptType::YesNo)
		wxPromptType = wxYES | wxNO;
	else if (promptType == PromptType::YesNoCancel)
		wxPromptType = wxYES | wxNO | wxCANCEL;

	int wxResult = wxMessageBox(wxString::FromAscii(message), wxString::FromAscii(title), wxPromptType);

	PromptResult::Type result = PromptResult::Cancel;
	if (wxResult == wxOK)
		result = PromptResult::Ok;
	else if (wxResult == wxCANCEL)
		result = PromptResult::Cancel;
	else if (wxResult == wxYES)
		result = PromptResult::Yes;
	else if (wxResult == wxNO)
		result = PromptResult::No;

	return result;
}

bool Application::SelectFile(char** result, const char* fileMask) {
	if (result == nullptr)
		return false;

	if (fileMask == nullptr)
		fileMask = "*.*|*.*";

	wxFileDialog openFileDialog(nullptr, _("Open File"), _(""), _(""), wxString::FromAscii(fileMask),
								wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (openFileDialog.ShowModal() == wxID_CANCEL)
		return false;

	int bufferSize = openFileDialog.GetPath().Length() + 1;

	*result = (char*)malloc(bufferSize);
	strcpy(*result, openFileDialog.GetPath().ToAscii());

	return true;
}

void Application::ConsolePrint(const char* text) {
	m_consoleWindow->ConsolePrint(text);
}

unsigned int Application::GetRomDataSize(const char* title) {
	return 0;
}

// IWindowMessageListener

void Application::Closed() {
}

void Application::Draw() {
	if (m_renderer != nullptr)
		m_renderer->Draw();
}

void Application::Resize(int newWidth, int newHeight) {
	if (m_renderer != nullptr)
		m_renderer->Resize(newWidth, newHeight);
}

void Application::KeyDown(int key) {
	char* fileSelected = nullptr;

	if (key >= 'a' && key <= 'z')
		key = 'A' + (key - 'a');

	if (key == 'Q' || key == WXK_ESCAPE)
		RequestShutdown();

	else if (key == 'T')
		DisplayImportantMessage(MessageType::Information, "test");

	else if (key == '`')
		ShowConsoleWindow();

	else if (key == 'O') {
		SelectFile(&fileSelected, nullptr);

		if (fileSelected != nullptr) {
			bool result = LoadRom(fileSelected);
			if (result == false)
				DisplayImportantMessage(MessageType::Error, "Failed to load the specified file.");

			free(fileSelected);
		}
	}

	else {
		m_inputManager->KeyDown(key);
	}
}

void Application::KeyUp(int key) {
	if (key >= 'a' && key <= 'z')
		key = 'A' + (key - 'a');

	m_inputManager->KeyUp(key);
}

// BaseApplication interface

Archive* Application::OpenRomData(const char* name, bool saving) {
	return OpenFileArchive(GetRomDataFile(name).c_str(), saving);
}

void Application::CloseRomData(Archive* archive) {
	ReleaseArchive(archive);
}

Archive* Application::OpenSavestate(const char* name, bool saving) {
	return OpenFileArchive(GetSaveStateFile(name).c_str(), saving);
}

void Application::CloseSavestate(Archive* archive) {
	ReleaseArchive(archive);
}

Archive* Application::OpenMovie(const char* name, bool saving) {
	return OpenFileArchive(GetMovieFile(name).c_str(), saving);
}

void Application::CloseMovie(Archive* archive) {
	ReleaseArchive(archive);
}

Archive* Application::OpenMacro(const char* name, bool saving) {
	return OpenFileArchive(GetMacroFile(name).c_str(), saving);
}

void Application::CloseMacro(Archive* archive) {
	ReleaseArchive(archive);
}

//  Persistence

Archive* Application::OpenFileArchive(const char* filename, bool saving) {
	FileSerializer* serializer = new FileSerializer();
	serializer->SetFile(filename);

	ArchiveMode::Type mode;
	if (saving == true)
		mode = ArchiveMode::Saving;
	else
		mode = ArchiveMode::Loading;

	Archive* archive = new Archive(serializer, mode);
	return archive;
}

void Application::ReleaseArchive(Archive* archive) {
	if (archive == nullptr)
		return;

	ISerializer* serializer = archive->GetSerializer();

	archive->Close();

	delete archive;
	delete serializer;
}

std::string Application::GetDataFolder() {
#ifdef EMUNISCE_PLATFORM_WINDOWS
	// todo
	return std::string(".");

#elif EMUNISCE_PLATFORM_LINUX
	std::string result = getenv("HOME");
	if (result.length() == 0)
		result = ".";

	result += std::string("/.Emunisce");

	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	return result;

#endif
}

std::string Application::GetSaveStateFile(const char* name) {
#ifdef EMUNISCE_PLATFORM_WINDOWS
	// todo
	return GetDataFolder() + std::string("/") + std::string(name) + std::string(".ess");

#elif EMUNISCE_PLATFORM_LINUX
	std::string result = GetDataFolder();

	result += std::string("/SaveStates");
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(Machine::ToString[m_machine->GetType()]);
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(m_machine->GetRomTitle());
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(name) + std::string(".ess");

	return result;
#endif
}

std::string Application::GetRomDataFile(const char* name) {
#ifdef EMUNISCE_PLATFORM_WINDOWS
	// todo
	return GetDataFolder() + std::string("/") + std::string(name) + std::string(".erd");

#elif EMUNISCE_PLATFORM_LINUX
	std::string result = GetDataFolder();

	result += std::string("/RomData");
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(Machine::ToString[m_machine->GetType()]);
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(m_machine->GetRomTitle());
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(name) + std::string(".erd");

	return result;
#endif
}

std::string Application::GetMovieFile(const char* name) {
#ifdef EMUNISCE_PLATFORM_WINDOWS
	// todo
	return GetDataFolder() + std::string("/") + std::string(name) + std::string(".eim");

#elif EMUNISCE_PLATFORM_LINUX
	std::string result = GetDataFolder();

	result += std::string("/Movies");
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(Machine::ToString[m_machine->GetType()]);
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(m_machine->GetRomTitle());
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(name) + std::string(".eim");

	return result;
#endif
}

std::string Application::GetMacroFile(const char* name) {
#ifdef EMUNISCE_PLATFORM_WINDOWS
	// todo
	return GetDataFolder() + std::string("/") + std::string(name) + std::string(".eir");

#elif EMUNISCE_PLATFORM_LINUX
	std::string result = GetDataFolder();

	result += std::string("/Macros");
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	result += std::string("/") + std::string(Machine::ToString[m_machine->GetType()]);
	mkdir(result.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	// no rom title folder for macros.  they're global to the machine.

	result += std::string("/") + std::string(name) + std::string(".eir");

	return result;
#endif
}

// Input

void Application::MapDefaultKeys() {
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

bool Application::OnInit() {
	// Set up the main window and renderer frame

	m_frame = new wxFrame((wxFrame*)nullptr, -1, wxT("Emunisce"), wxPoint(50, 50), wxSize(320, 240));

	int args[] = {WX_GL_RGBA, WX_GL_DOUBLEBUFFER, WX_GL_DEPTH_SIZE, 16, 0};

	m_windowMain = new WindowMain(m_frame, args);
	m_windowMain->SetApplication(this);

	wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
	sizer->Add(m_windowMain, 1, wxEXPAND);

	m_frame->SetSizer(sizer);
	m_frame->SetAutoLayout(true);

	m_frame->Show();

	m_consoleWindow = new ConsoleWindow(this, m_frame);
	m_consoleWindow->GiveFocus();

	ConsolePrint("Welcome to Emunisce\n");
	ConsolePrint("===================\n");
	ConsolePrint("Press tilde (`) to switch focus between the console and the game\n");
	ConsolePrint("Type 'help' to see a list of commands\n");
	ConsolePrint("\n");

	// Initialize the renderer and hand off to the machine

	m_renderer->Initialize(nullptr);

	if (m_machine != nullptr) {
		BaseApplication::NotifyMachineChanged(m_machine);
		m_renderer->SetMachine(m_machine);
	}

	Run();

	return true;
}

void Application::ShowConsoleWindow() {
	m_consoleWindow->GiveFocus();
}

void Application::ShowGameWindow() {
	m_frame->Show();
	m_frame->Raise();

	m_windowMain->SetFocus();
}

bool Application::ExecuteConsoleCommand(const char* command) {
	return BaseApplication::ExecuteConsoleCommand(command);
}
