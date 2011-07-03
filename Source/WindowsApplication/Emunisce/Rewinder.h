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
#ifndef REWINDER_H
#define REWINDER_H

#include "MachineFeature.h"

#include "PlatformIncludes.h"

#include <list>
using namespace std;


namespace Emunisce
{

class Rewinder : public MachineFeature
{
public:

	// Rewinder

	Rewinder();
	virtual ~Rewinder();

	virtual void StartRewinding();
	virtual void StopRewinding();


	// IEmulatedMachine

	virtual void RunToNextFrame();


	// IEmulatedDisplay

	virtual ScreenBuffer* GetStableScreenBuffer();
	virtual int GetScreenBufferCount();


protected:

	bool m_isRewinding;

	struct FrameInfo
	{
		unsigned int Id;
		ScreenBuffer* Screen;
	};

	list<FrameInfo> m_frameHistory;
	list<FrameInfo>::iterator m_playbackFrame;
	static const unsigned int m_maxFrameHistorySize = 60;
	Mutex m_frameHistoryLock;


	class InputHandler : public IEmulatedInput
	{
	public:

		InputHandler(Rewinder* rewinder);


		//IEmulatedInput

		virtual unsigned int NumButtons();
		virtual const char* GetButtonName(unsigned int index);

		virtual void ButtonDown(unsigned int index);
		virtual void ButtonUp(unsigned int index);


	private:

		Rewinder* m_rewinder;
	};

	InputHandler* m_inputHandler;
};

}	//namespace Emunisce

#endif
