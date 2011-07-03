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

#include "UserInterface.h"	///<For DisplayFilter


namespace Emunisce
{

class Window;

class Gui;
class Rewinder;
class InputRecording;

class IEmulatedMachine;

class MachineRunner;
class ConsoleDebugger;

class GdiPlusRenderer;
class KeyboardInput;
class WaveOutSound;


class EmunisceApplication
{
public:

	EmunisceApplication();
	~EmunisceApplication();

	void RunWindow();	///<Pumps messages on the window until shutdown is requested.  Blocks until shutdown.

	bool LoadRom(const char* filename);
	void ResetRom();

	void SaveState(const char* id);
	void LoadState(const char* id);

	void EnableBackgroundAnimation();
	void DisableBackgroundAnimation();

	void SetDisplayFilter(DisplayFilter::Type filter);
	void SetVsync(bool enabled);

	void StartRecordingInput();
	void StopRecordingInput();

	void PlayMovie();
	void StopMovie();

	void SaveMovie(const char* id);
	void LoadMovie(const char* id);

	void PlayMacro(bool loop);
	void StopMacro();

	void SaveMacro(const char* id);
	void LoadMacro(const char* id);

	void NotifyMachineChanged(IEmulatedMachine* newMachine);
	IEmulatedMachine* GetMachine();

	bool ShutdownRequested();
	void RequestShutdown();

	Window* GetWindow();

	Gui* GetGui();
	Rewinder* GetRewinder();
	InputRecording* GetInputRecorder();

	UserInterface* GetUserInterface();

	MachineRunner* GetMachineRunner();
	ConsoleDebugger* GetDebugger();

	GdiPlusRenderer* GetRenderer();
	KeyboardInput* GetInput();
	WaveOutSound* GetSound();

private:

	class Phoenix_Private* m_private;
};

}	//namespace Emunisce

#endif
