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

typedef TScreenBuffer<320, 240> GuiScreenBuffer;

class Gui : public MachineFeature
{
public:

	// Gui

	Gui();
	virtual ~Gui();


	// IEmulatedDisplay

	virtual ScreenBuffer* GetStableScreenBuffer();
	virtual void SetFilter(DisplayFilter::Type filter);


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


		// IExecutableFeature

		virtual unsigned int GetFrameCount();
		virtual unsigned int GetTicksPerSecond();
		virtual unsigned int GetTicksUntilNextFrame();

		virtual void Step();
		virtual void RunToNextFrame();


		// IEmulatedDisplay

		virtual ScreenResolution GetScreenResolution();
		virtual ScreenBuffer* GetStableScreenBuffer();
		virtual int GetScreenBufferCount();

		virtual void SetFilter(DisplayFilter::Type filter);


		// IEmulatedInput

		virtual unsigned int NumButtons();
		virtual const char* GetButtonName(unsigned int index);

		virtual void ButtonDown(unsigned int index);
		virtual void ButtonUp(unsigned int index);


	protected:

		int m_ticksPerFrame;
		int m_ticksUntilNextFrame;
		int m_frameCount;

		GuiScreenBuffer m_screenBuffer;

		
		static const int m_numAttractors = 5;	///<Arbitrary constant.  How many frames to blend together.
		GuiScreenBuffer m_attractorBuffer[m_numAttractors];
		int m_currentAttractorBuffer;

		float m_x, m_y;
		float m_a, m_b, m_c, m_d;
		int m_framesThisAttractor;

		inline void SilentDream();
		inline void Dream();
	};

	GuiFeature* m_guiFeature;
};

}	//namespace Emunisce

#endif
