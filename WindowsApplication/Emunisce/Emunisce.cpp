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

#include "Emunisce.h"
using namespace Emunisce;

#include "windows.h"
#include "shlobj.h"
#include "shlwapi.h"

#include "PlatformIncludes.h"

#include "MachineIncludes.h"

#include "BaseApplication/InputManager.h"
#include "BaseApplication/MachineFeature.h"

#include "Serialization/SerializationIncludes.h"
#include "Serialization/FileSerializer.h"

#include "ConsoleDebugger.h"





EmunisceApplication::EmunisceApplication()
{
	m_window = new Window();

	m_debugger = new ConsoleDebugger();

	//_Renderer = new GdiPlusRenderer();
	m_renderer = new OpenGLRenderer();
	m_sound = new WaveOutSound();

	AdjustWindowSize();

	m_window->Create(320, 240, "Emunisce", "Emunisce_RenderWindow");
	m_window->Show();
	m_window->SubscribeListener(this);

	m_debugger->Initialize(this);

	m_renderer->Initialize(m_window->GetHandle());
	m_sound->Initialize(this);

	MapDefaultKeys();

	//Calling HandlePendingMachineChange here with just the MachineFeature components
	//(no emulated machine yet) so we don't have to force a LoadROM immediately.
	if(m_machine != nullptr)
	{
		m_pendingMachine = m_machine;
		HandlePendingMachineChange();
	}
}

EmunisceApplication::~EmunisceApplication()
{
	m_window->UnsubscribeListener(this);
	m_window->Destroy();

	m_sound->Shutdown();
	m_renderer->Shutdown();

	m_debugger->Shutdown();

	delete m_sound;
	delete m_renderer;

	delete m_debugger;

	delete m_window;
}

void EmunisceApplication::RunWindow()
{
	while(ShutdownRequested() == false)
	{
		HandlePendingMachineChange();

		IEmulatedMachine* machine = GetMachine();
		if(machine)
		{
			if(m_renderer->GetLastFrameRendered() != machine->GetDisplay()->GetScreenBufferCount() && ShutdownRequested() == false)
			{
				HWND hwnd = (HWND)GetWindow()->GetHandle();
				RECT clientRect;

				GetClientRect(hwnd, &clientRect);
				InvalidateRect(hwnd, &clientRect, true);
				UpdateWindow(hwnd);

				GetWindow()->PumpMessages();
			}

			else if(ShutdownRequested() == false)
			{
				GetWindow()->PumpMessages();
				Sleep(15);
			}
		}
		else
		{
			GetWindow()->PumpMessages();
			Sleep(100);
		}
	}
}

Window* EmunisceApplication::GetWindow()
{
	return m_window;
}


ConsoleDebugger* EmunisceApplication::GetDebugger()
{
	return m_debugger;
}

WaveOutSound* EmunisceApplication::GetSound()
{
	return m_sound;
}


void EmunisceApplication::NotifyMachineChanged(IEmulatedMachine* newMachine)
{
	//RunWindow must handle machine changes (rendering things have to happen on that thread)
	m_pendingMachine = newMachine;
	while(m_pendingMachine != nullptr)
		Sleep(10);
}

void EmunisceApplication::RequestShutdown()
{
	BaseApplication::RequestShutdown();

	if(m_window != nullptr)
		m_window->RequestExit();
}


void EmunisceApplication::SetVsync(bool enabled)
{
	if(m_renderer != nullptr)
		m_renderer->SetVsync(enabled);
}


void EmunisceApplication::DisplayStatusMessage(const char* /*message*/)
{
	//todo
}

void EmunisceApplication::DisplayImportantMessage(MessageType::Type messageType, const char* message)
{
	int iconType = 0;
	if(messageType == MessageType::Information)
		iconType = MB_ICONINFORMATION;
	else if(messageType == MessageType::Warning)
		iconType = MB_ICONWARNING;
	else if(messageType == MessageType::Error)
		iconType = MB_ICONERROR;

	MessageBox(nullptr, message, "Phoenix", iconType | MB_OK);
}

PromptResult::Type EmunisceApplication::DisplayPrompt(PromptType::Type promptType, const char* title, const char* message, void** /*extraResult*/)
{
	int windowsPromptType = MB_OK;
	if(promptType == PromptType::OkCancel)
		windowsPromptType = MB_OKCANCEL;
	else if(promptType == PromptType::YesNo)
		windowsPromptType = MB_YESNO;
	else if(promptType == PromptType::YesNoCancel)
		windowsPromptType = MB_YESNOCANCEL;

	int windowsResult = MessageBox(nullptr, message, title, windowsPromptType);

	PromptResult::Type result = PromptResult::Cancel;
	if(windowsResult == IDOK)
		result = PromptResult::Ok;
	else if(windowsResult == IDCANCEL)
		result = PromptResult::Cancel;
	else if(windowsResult == IDYES)
		result = PromptResult::Yes;
	else if(windowsResult == IDNO)
		result = PromptResult::No;

	return result;
}


bool EmunisceApplication::SelectFile(char** result, const char* fileMask)
{
	if(result == nullptr)
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

void EmunisceApplication::ConsolePrint(const char* /*text*/)
{
}

unsigned int EmunisceApplication::GetRomDataSize(const char* title)
{
	std::string filename = GetCurrentRomDataFile(title);

	std::ifstream ifile;
	ifile.open(filename.c_str(), std::ios::in | std::ios::binary);

	if(ifile.good() == false)
		return 0;

	ifile.seekg(0, std::ios::beg);
	unsigned int beginPosition = (int)ifile.tellg();

	ifile.seekg(0, std::ios::end);
	unsigned int endPosition = (int)ifile.tellg();

	return endPosition - beginPosition;
}


// IWindowMessageListener

void EmunisceApplication::Closed()
{
	RequestShutdown();
}

void EmunisceApplication::Draw()
{
	if(m_renderer != nullptr)
		m_renderer->Draw();
}

void EmunisceApplication::Resize(int newWidth, int newHeight)
{
	if(m_renderer != nullptr)
		m_renderer->Resize(newWidth, newHeight);

	AdjustWindowSize();
}

void EmunisceApplication::KeyDown(int key)
{
	m_inputManager->KeyDown(key);
}

void EmunisceApplication::KeyUp(int key)
{
	m_inputManager->KeyUp(key);
}


void EmunisceApplication::AdjustWindowSize()
{
	//Resize the window so that the client area is a whole multiple of the display resolution

	if(m_machine == nullptr)
		return;

	if(m_window == nullptr)
		return;

	HWND windowHandle = (HWND)m_window->GetHandle();

	ScreenResolution resolution = m_machine->GetDisplay()->GetScreenResolution();
	int nativeWidth = resolution.width;
	int nativeHeight = resolution.height;

	RECT clientRect;
	GetClientRect(windowHandle, &clientRect);
	int clientWidth = clientRect.right - clientRect.left;
	int clientHeight = clientRect.bottom - clientRect.top;

	if( (clientWidth % nativeWidth) != 0 || (clientHeight % nativeHeight) != 0 )
	{
		//Figure out what the new client area should be.
		// Adjust to the nearest multiple.
		// Assuming the native width is 160...
		//  If we're at 161, we want to decrease to 160
		//  But if we're at 319, we want to increase to 320

		int newWidth = clientWidth;
		if(clientWidth < nativeWidth)		///<Only support 1x scale or greater for now
			newWidth = nativeWidth;
		else if(clientWidth % nativeWidth < 80)
			newWidth = clientWidth - (clientWidth % nativeWidth);	///<Nearest multiple is smaller than the current width
		else
			newWidth = clientWidth + (nativeWidth - (clientWidth % nativeWidth));	///<Nearest multiple is larger than the current width

		int newHeight = nativeHeight * (newWidth / nativeWidth);


		//Figure out what we need to add to the original size in order to get our new target size

		int deltaWidth = newWidth - clientWidth;
		int deltaHeight = newHeight - clientHeight;


		//Apply the deltas to the window size
		// The client size is the drawable area
		// The window size is the drawable area plus the title bar, borders, etc

		RECT windowRect;
		GetWindowRect(windowHandle, &windowRect);
		windowRect.right += deltaWidth;
		windowRect.bottom += deltaHeight;

		MoveWindow(windowHandle, windowRect.left, windowRect.top, (windowRect.right - windowRect.left), (windowRect.bottom - windowRect.top), TRUE);
	}
}

void EmunisceApplication::HandlePendingMachineChange()
{
	if(m_pendingMachine == nullptr)
		return;

	IEmulatedMachine* newMachine = m_pendingMachine;

	BaseApplication::NotifyMachineChanged(newMachine);

	m_debugger->SetMachine(m_machine);

	m_renderer->SetMachine(m_machine);
	m_sound->SetMachine(m_machine);

	//Start out at the native resolution or 320x240 (adjusted for aspect ratio), whichever is larger.
	ScreenResolution resolution = m_machine->GetScreenResolution();

	WindowSize size;
	size.width = 320;
	size.height = 240;

	if(resolution.width >= 320)
	{
		size.width = resolution.width;
		size.height = resolution.height;
	}

	m_window->SetSize(size);

	//Adjust to make up for aspect ratio, borders, titlebar, menu, etc
	AdjustWindowSize();

	m_pendingMachine = nullptr;
}


Archive* EmunisceApplication::OpenFileArchive(const char* filename, bool saving)
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

void EmunisceApplication::ReleaseArchive(Archive* archive)
{
	if(archive == nullptr)
		return;

	ISerializer* serializer = archive->GetSerializer();

	archive->Close();

	delete archive;
	delete serializer;
}


Archive* EmunisceApplication::OpenRomData(const char* name, bool saving)
{
	return OpenFileArchive( GetCurrentRomDataFile(name).c_str(), saving );
}

void EmunisceApplication::CloseRomData(Archive* archive)
{
	ReleaseArchive(archive);
}


Archive* EmunisceApplication::OpenSavestate(const char* name, bool saving)
{
	return OpenFileArchive( GetCurrentSaveStateFile(name).c_str(), saving );
}

void EmunisceApplication::CloseSavestate(Archive* archive)
{
	ReleaseArchive(archive);
}


Archive* EmunisceApplication::OpenMovie(const char* name, bool saving)
{
	return OpenFileArchive( GetCurrentMovieFile(name).c_str(), saving );
}

void EmunisceApplication::CloseMovie(Archive* archive)
{
	ReleaseArchive(archive);
}


Archive* EmunisceApplication::OpenMacro(const char* name, bool saving)
{
	return OpenFileArchive( GetCurrentMacroFile(name).c_str(), saving );
}

void EmunisceApplication::CloseMacro(Archive* archive)
{
	ReleaseArchive(archive);
}


std::string EmunisceApplication::GetDataFolder()
{
	char path[MAX_PATH];

	SHGetFolderPath(nullptr, CSIDL_LOCAL_APPDATA | CSIDL_FLAG_CREATE, nullptr, 0, path);

	PathAppend(path, "Emunisce");

	SHCreateDirectoryEx(nullptr, path, nullptr);

	return std::string(path);
}


std::string EmunisceApplication::GetBaseSaveStateFolder()
{
	char path[MAX_PATH];

	std::string dataFolder = GetDataFolder();
	strcpy_s(path, MAX_PATH, dataFolder.c_str());

	PathAppend(path, "SaveStates");

	SHCreateDirectoryEx(nullptr, path, nullptr);

	return std::string(path);
}

std::string EmunisceApplication::GetCurrentSaveStateFolder()
{
	char path[MAX_PATH] = {0};

	std::string basePath = GetBaseSaveStateFolder();
	strcpy_s(path, MAX_PATH, basePath.c_str());

	PathAppend(path, EmulatedMachine::ToString[ m_machine->GetType() ]);
	PathAppend(path, m_machine->GetRomTitle());

	SHCreateDirectoryEx(nullptr, path, nullptr);

	return std::string(path);
}

std::string EmunisceApplication::GetCurrentSaveStateFile(const char* id)
{
	char file[MAX_PATH] = {0};

	std::string path = GetCurrentSaveStateFolder();
	strcpy_s(file, MAX_PATH, path.c_str());

	char idStr[MAX_PATH];
	sprintf_s(idStr, MAX_PATH, "%s.ess", id);

	PathAppend(file, idStr);

	return std::string(file);
}


std::string EmunisceApplication::GetBaseRomDataFolder()
{
	char path[MAX_PATH];

	std::string dataFolder = GetDataFolder();
	strcpy_s(path, MAX_PATH, dataFolder.c_str());

	PathAppend(path, "RomData");

	SHCreateDirectoryEx(nullptr, path, nullptr);

	return std::string(path);
}

std::string EmunisceApplication::GetCurrentRomDataFolder()
{
	char path[MAX_PATH] = {0};

	std::string basePath = GetBaseRomDataFolder();
	strcpy_s(path, MAX_PATH, basePath.c_str());

	PathAppend(path, EmulatedMachine::ToString[ m_machine->GetType() ]);
	PathAppend(path, m_machine->GetRomTitle());

	SHCreateDirectoryEx(nullptr, path, nullptr);

	return std::string(path);
}

std::string EmunisceApplication::GetCurrentRomDataFile(const char* name)
{
	char file[MAX_PATH] = {0};

	std::string path = GetCurrentRomDataFolder();
	strcpy_s(file, MAX_PATH, path.c_str());

	std::string filename = std::string(name) + std::string(".erd");
	PathAppend(file, filename.c_str());

	return std::string(file);
}


std::string EmunisceApplication::GetBaseMovieFolder()
{
	char path[MAX_PATH];

	std::string dataFolder = GetDataFolder();
	strcpy_s(path, MAX_PATH, dataFolder.c_str());

	PathAppend(path, "Movies");

	SHCreateDirectoryEx(nullptr, path, nullptr);

	return std::string(path);
}

std::string EmunisceApplication::GetCurrentMovieFolder()
{
	char path[MAX_PATH] = {0};

	std::string basePath = GetBaseMovieFolder();
	strcpy_s(path, MAX_PATH, basePath.c_str());

	PathAppend(path, EmulatedMachine::ToString[ m_machine->GetType() ]);
	PathAppend(path, m_machine->GetRomTitle());

	SHCreateDirectoryEx(nullptr, path, nullptr);

	return std::string(path);
}

std::string EmunisceApplication::GetCurrentMovieFile(const char* name)
{
	char file[MAX_PATH] = {0};

	std::string path = GetCurrentMovieFolder();
	strcpy_s(file, MAX_PATH, path.c_str());

	std::string filename = std::string(name) + std::string(".eim");
	PathAppend(file, filename.c_str());

	return std::string(file);
}


std::string EmunisceApplication::GetBaseMacroFolder()
{
	char path[MAX_PATH];

	std::string dataFolder = GetDataFolder();
	strcpy_s(path, MAX_PATH, dataFolder.c_str());

	PathAppend(path, "Macros");

	SHCreateDirectoryEx(nullptr, path, nullptr);

	return std::string(path);
}

std::string EmunisceApplication::GetCurrentMacroFolder()
{
	return GetBaseMacroFolder();
}

std::string EmunisceApplication::GetCurrentMacroFile(const char* name)
{
	char file[MAX_PATH] = {0};

	std::string path = GetCurrentMacroFolder();
	strcpy_s(file, MAX_PATH, path.c_str());

	std::string filename = std::string(name) + std::string(".eir");
	PathAppend(file, filename.c_str());

	return std::string(file);
}


void EmunisceApplication::MapDefaultKeys()
{
	m_inputManager->MapKey("Up", VK_UP);
	m_inputManager->MapKey("Down", VK_DOWN);
	m_inputManager->MapKey("Left", VK_LEFT);
	m_inputManager->MapKey("Right", VK_RIGHT);

	m_inputManager->MapKey("B", 'Q');
	m_inputManager->MapKey("B", 'A');
	m_inputManager->MapKey("B", 'Z');

	m_inputManager->MapKey("A", 'W');
	m_inputManager->MapKey("A", 'S');
	m_inputManager->MapKey("A", 'X');

	m_inputManager->MapKey("Select", 'V');
	m_inputManager->MapKey("Start", 'B');

	m_inputManager->MapKey("Select", VK_LSHIFT);
	m_inputManager->MapKey("Select", VK_RSHIFT);
	m_inputManager->MapKey("Start", VK_RETURN);

	m_inputManager->MapKey("Select", VK_OEM_4);
	m_inputManager->MapKey("Start", VK_OEM_6);

	m_inputManager->MapKey("Rewind", VK_TAB);
}
