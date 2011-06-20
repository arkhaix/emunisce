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
#ifndef GUI_H
#define GUI_H

#include "MachineFeature.h"

#include "MachineIncludes.h"

namespace Emunisce
{

namespace GuiButtons
{
	typedef int Type;

	enum
	{
		NumGuiButtons
	};

	static const char* ToString[] =
	{
		"NumGuiButtons"
	};
}

typedef TScreenBuffer<640, 480> GuiScreenBuffer;

class Gui : public MachineFeature
{
public:

	// Gui

	Gui();
	virtual ~Gui();


protected:

	class GuiDisplay : public IEmulatedDisplay
	{
	public:

		// IEmulatedDisplay

		virtual ScreenResolution GetScreenResolution();
		virtual ScreenBuffer* GetStableScreenBuffer();
		virtual int GetScreenBufferCount();

		virtual void SetFilter(DisplayFilter::Type filter);


	protected:

		GuiScreenBuffer m_screenBuffer;
		int m_screenBufferCount;
	};

	GuiDisplay* m_guiDisplay;


	class GuiInput : public IEmulatedInput
	{
	public:

		// IEmulatedInput

		virtual unsigned int NumButtons();
		virtual const char* GetButtonName(unsigned int index);

		virtual void ButtonDown(unsigned int index);
		virtual void ButtonUp(unsigned int index);

		
	protected:

	};

	GuiInput* m_guiInput;
};

}	//namespace Emunisce

#endif
