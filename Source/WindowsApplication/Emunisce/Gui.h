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

#include "UserInterface.h"	///<For DisplayFilter

namespace Emunisce
{

class KingsDream;

namespace GuiButtons
{
	typedef int Type;

	enum
	{
		NumGuiButtons
	};

#ifdef GuiButtons_ToString
	static const char* ToString[] =
	{
		"NumGuiButtons"
	};
#endif
}

typedef TScreenBuffer<320, 240> GuiScreenBuffer;

class Gui : public MachineFeature
{
public:

	// Gui

	Gui();
	virtual ~Gui();

	void EnableBackgroundAnimation();
	void DisableBackgroundAnimation();

	virtual void SetDisplayFilter(DisplayFilter::Type filter);


	// IEmulatedDisplay

	virtual ScreenBuffer* GetStableScreenBuffer();


protected:

    //Gui properties

	DynamicScreenBuffer* m_screenBufferCopy;
	DisplayFilter::Type m_screenBufferCopyFilter;
	//Mutex m_screenBufferLock;

    ScreenBuffer* m_filteredScreenBuffer;
    int m_filteredScreenBufferId;
    DisplayFilter::Type m_displayFilter;


    // GuiFeature

	class GuiFeature : public IExecutableFeature, public IEmulatedDisplay, public IEmulatedInput
	{
	public:

		// GuiFeature

		GuiFeature();

		void EnableBackgroundAnimation();
		void DisableBackgroundAnimation();


		// IExecutableFeature

		virtual unsigned int GetFrameCount();
		virtual unsigned int GetTickCount();
		virtual unsigned int GetTicksPerSecond();
		virtual unsigned int GetTicksUntilNextFrame();

		virtual void Step();
		virtual void RunToNextFrame();


		// IEmulatedDisplay

		virtual ScreenResolution GetScreenResolution();
		virtual ScreenBuffer* GetStableScreenBuffer();
		virtual int GetScreenBufferCount();


		// IEmulatedInput

		virtual unsigned int NumButtons();
		virtual const char* GetButtonName(unsigned int index);

		virtual void ButtonDown(unsigned int index);
		virtual void ButtonUp(unsigned int index);

		virtual bool IsButtonDown(unsigned int index);


	protected:

		GuiScreenBuffer m_screenBuffer;

		int m_ticksThisFrame;
		int m_ticksPerFrame;
		int m_frameCount;

		bool m_backgroundEnabled;
		KingsDream* m_backgroundAnimation;
	};

	GuiFeature* m_guiFeature;
};

}	//namespace Emunisce

#endif
