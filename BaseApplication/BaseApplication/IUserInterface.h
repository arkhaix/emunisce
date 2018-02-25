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
#ifndef IUSERINTERFACE_H
#define IUSERINTERFACE_H


namespace Emunisce
{

namespace DisplayFilter
{
	typedef int Type;

	enum
	{
		NoFilter = 0,   ///<Was 'None', but changed due to a name conflict

		Hq2x,
		Hq3x,
		Hq4x,

		NumDisplayFilters
	};
}

namespace MessageType
{
	typedef int Type;

	enum
	{
		Unspecified,

		Information,
		Warning,
		Error,

		NumMessageTypes
	};
}

namespace PromptType
{
	typedef int Type;

	enum
	{
		Ok,
		OkCancel,

		YesNo,
		YesNoCancel,

		NumPromptTypes
	};
}

namespace PromptResult
{
	typedef int Type;

	enum
	{
		Cancel,

		Ok,

		Yes,
		No,

		NumPromptResults
	};

#ifdef PromptResult_ToString
	static const char* ToString[] =
	{
		"Cancel",

		"Ok",

		"Yes",
		"No",

		"NumPromptResults"
	};
#endif
}


class IUserInterface
{
public:


	// User to application

	//Rom
	virtual bool LoadRom(const char* filename) = 0;	///<Attempts to load the specified rom.  Returns true on success, false on failure.
	virtual void Reset() = 0;	///<Reloads the last successfully loaded rom and resets the machine state to the beginning.

	//Emulation
	virtual void SetEmulationSpeed(float multiplier) = 0;	///<1.0 = normal, 0.5 = half normal, 2.0 = twice normal, any value less than or equal to 0 = no throttle (max speed)

	virtual void Run() = 0;	///<Runs the machine at the speed defined by SetEmulationSpeed.
	virtual void Pause() = 0;	///<Pauses the machine.  Preserves the SetEmulationSpeed setting.

	virtual void StepInstruction() = 0;	///<Pauses if necessary, then steps forward one cpu instruction.
	virtual void StepFrame() = 0;	///<Pauses if necessary, then steps forward 1/60th of a second.

	//State
	virtual void SaveState(const char* id) = 0;	///<Requires that a rom is loaded.  Saves state basd on rom name.  id is a slot identifier (any value is okay).
	virtual void LoadState(const char* id) = 0;	///<Requires that a rom is loaded.  Loads state based on rom name and specified slot id.

	//Gui
	virtual void EnableBackgroundAnimation() = 0;
	virtual void DisableBackgroundAnimation() = 0;

	//Display
	virtual void SetDisplayFilter(DisplayFilter::Type displayFilter) = 0;	///<Sets the desired display filter.
	virtual void SetVsync(bool enabled) = 0;	///<Enabled = true enables vsync if possible.  Not all renderers support this.  Enabled by default.

	//Input movie
	virtual void StartRecordingInput() = 0;
	virtual void StopRecordingInput() = 0;

	virtual void PlayMovie() = 0;
	virtual void StopMovie() = 0;

	virtual void SaveMovie(const char* id) = 0;	///<Requires that a rom is loaded.  Saves movie based on rom name + id.
	virtual void LoadMovie(const char* id) = 0;	///<Requires that a rom is loaded.  Loads movie based on rom name + id.

	virtual void PlayMacro(bool loop) = 0;
	virtual void StopMacro() = 0;

	virtual void SaveMacro(const char* id) = 0;
	virtual void LoadMacro(const char* id) = 0;


	//Application to user

	virtual void DisplayStatusMessage(const char* message) = 0;	///<Updates a global status element.  Use for occasionally informing the user of non-essential information.  No acknowledgement required.
	virtual void DisplayImportantMessage(MessageType::Type messageType, const char* message) = 0;	///<Displays a message to the user and blocks until the message is acknowledged.
	virtual PromptResult::Type DisplayPrompt(PromptType::Type promptType, const char* title, const char* message, void** extraResult) = 0;	///<Displays a prompt the user and blocks until the prompt is answered.  extraResult is for responses from the user that can't be returned as PromptResult values.

	virtual bool SelectFile(char** result, const char* fileMask = 0) = 0;	///<Prompts the user to select a file.  Allocates result, sets it to the absolute path to the file, and returns true on success.  On cancellation, failure, or an invalid file selection, result is unchanged and the function returns false.

    virtual void ConsolePrint(const char* text) = 0;    ///<Displays a message in the console.
};

}	//namespace Emunisce

#endif
