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
#ifndef USERINTERFACE_H
#define USERINTERFACE_H



namespace Emunisce
{

class Phoenix;
class MachineRunner;

class IEmulatedMachine;


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

	static const char* ToString[] =
	{
		"Cancel",

		"Ok",
		
		"Yes",
		"No",

		"NumPromptResults"
	};
}


class UserInterface
{
public:

	// Application component

	UserInterface();
	virtual ~UserInterface();

	virtual void Initialize(Phoenix* phoenix);
	virtual void Shutdown();

	virtual void SetMachine(IEmulatedMachine* machine);


	// User to application

	//Rom
	virtual bool LoadRom(const char* filename);	///<Attempts to load the specified rom.  Returns true on success, false on failure.
	virtual void Reset();	///<Reloads the last successfully loaded rom and resets the machine state to the beginning.

	//Emulation
	virtual void SetEmulationSpeed(float multipler);	///<1.0 = normal, 0.5 = half normal, 2.0 = twice normal, any value less than or equal to 0 = no throttle (max speed)

	virtual void Run();	///<Runs the machine at the speed defined by SetEmulationSpeed.
	virtual void Pause();	///<Pauses the machine.  Preserves the SetEmulationSpeed setting.

	virtual void StepInstruction();	///<Pauses if necessary, then steps forward one cpu instruction.
	virtual void StepFrame();	///<Pauses if necessary, then steps forward 1/60th of a second.


	//Application to user

	virtual void DisplayStatusMessage(const char* message);	///<Updates a global status element.  Use for occasionally informing the user of non-essential information.  No acknowledgement required.
	virtual void DisplayImportantMessage(MessageType::Type messageType, const char* message);	///<Displays a message to the user and blocks until the message is acknowledged.
	virtual PromptResult::Type DisplayPrompt(PromptType::Type promptType, const char* title, const char* message, void** extraResult);	///<Displays a prompt the user and blocks until the prompt is answered.  extraResult is for responses from the user that can't be returned as PromptResult values.

	virtual bool SelectFile(char** result, const char* fileMask = 0);	///<Prompts the user to select a file.  Allocates result, sets it to the absolute path to the file, and returns yet on success.  On cancellation, failure, or an invalid file selection, result is unchanged and the function returns false.


protected:

	Phoenix* m_phoenix;
	MachineRunner* m_runner;
};

}	//namespace Emunisce

#endif
