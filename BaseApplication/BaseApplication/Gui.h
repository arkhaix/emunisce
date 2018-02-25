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

#include "IUserInterface.h"	///<For DisplayFilter

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
	~Gui() override;

	void EnableBackgroundAnimation();
	void DisableBackgroundAnimation();

	virtual void SetDisplayFilter(DisplayFilter::Type filter);


	// IEmulatedDisplay

	ScreenBuffer* GetStableScreenBuffer() override;


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
		virtual ~GuiFeature() = default;

		void EnableBackgroundAnimation();
		void DisableBackgroundAnimation();


		// IExecutableFeature

		unsigned int GetFrameCount() override;
		unsigned int GetTickCount() override;
		unsigned int GetTicksPerSecond() override;
		unsigned int GetTicksUntilNextFrame() override;

		void Step() override;
		void RunToNextFrame() override;


		// IEmulatedDisplay

		ScreenResolution GetScreenResolution() override;
		ScreenBuffer* GetStableScreenBuffer() override;
		int GetScreenBufferCount() override;


		// IEmulatedInput

		unsigned int NumButtons() override;
		const char* GetButtonName(unsigned int index) override;

		void ButtonDown(unsigned int index) override;
		void ButtonUp(unsigned int index) override;

		bool IsButtonDown(unsigned int index) override;


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
