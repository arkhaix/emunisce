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
#ifndef IMACHINETOUSER_H
#define IMACHINETOUSER_H

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

class IMachineToUser
{
public:

	virtual void DisplayStatusMessage(const char* message) = 0;	///<Updates a global status element.  Use for occasionally informing the user of non-essential information.  No acknowledgement required.

	virtual void DisplayImportantMessage(const char* message) = 0;	///<Displays a message to the user and blocks until the message is acknowledged.

	virtual PromptResult::Type DisplayPrompt(PromptType::Type promptType, const char* title, const char* message, void** extraResult) = 0;	///<Displays a prompt the user and blocks until the prompt is answered.  extraResult is for responses from the user that can't be returned as PromptResult values.

	virtual const char* SelectFile(const char* title, const char* fileMask) = 0;	///<Prompts the user to select a file.  Returns the absolute path to the file on success.  Returns null if the prompt fails, the user cancels the prompt, or the user supplies an invalid file.  If the result is non-null, then the file is guaranteed to exist.
};

#endif
