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
#include "Rewinder.h"
using namespace Emunisce;

#include <stdio.h>	///<printf debug


// Rewinder::InputHandler

Rewinder::InputHandler::InputHandler(Rewinder* rewinder)
{
	m_rewinder = rewinder;
}


//IEmulatedInput

namespace RewinderButtons
{
	typedef int Type;

	enum
	{
		Rewind = 0,

		NumButtons
	};

	static const char* ToString[] =
	{
		"Rewind",

		"NumButtons"
	};
}

unsigned int Rewinder::InputHandler::NumButtons()
{
	return RewinderButtons::NumButtons;
}

const char* Rewinder::InputHandler::GetButtonName(unsigned int index)
{
	if(index >= RewinderButtons::NumButtons)
		return NULL;

	return RewinderButtons::ToString[index];
}


void Rewinder::InputHandler::ButtonDown(unsigned int index)
{
	if(index == RewinderButtons::Rewind)
		m_rewinder->StartRewinding();
}

void Rewinder::InputHandler::ButtonUp(unsigned int index)
{
	if(index == RewinderButtons::Rewind)
		m_rewinder->StopRewinding();
}



// Rewinder

Rewinder::Rewinder()
{
	m_inputHandler = new Rewinder::InputHandler(this);
	m_featureInput = m_inputHandler;

	m_playbackFrame = m_frameHistory.end();

	m_isRewinding = false;
}

Rewinder::~Rewinder()
{
	m_isRewinding = false;

	while(m_frameHistory.empty() == false)
	{
		FrameInfo info = *m_frameHistory.begin();
		delete info.Screen;
		m_frameHistory.pop_front();
	}

	m_featureInput = NULL;
	delete m_inputHandler;
}


void Rewinder::StartRewinding()
{
	ScopedMutex scopedLock(m_frameHistoryLock);

	printf("StartRewinding\n");

	m_isRewinding = true;
	m_playbackFrame = m_frameHistory.end();
}

void Rewinder::StopRewinding()
{
	ScopedMutex scopedLock(m_frameHistoryLock);

	printf("StopRewinding\n");

	m_isRewinding = false;

	//We're altering the past.  We've created an alternate universe and caused the old future to vanish!
	//Works, but commented out until we can actually restore from the past position.
	/*
	auto deleteFrom = m_playbackFrame;

	while(m_playbackFrame != m_frameHistory.end())
	{
		delete m_playbackFrame->Screen;
		m_playbackFrame++;
	}
	m_frameHistory.erase(deleteFrom, m_frameHistory.end());

	m_playbackFrame = m_frameHistory.end();
	*/
}



// IEmulatedMachine

void Rewinder::RunToNextFrame()
{
	ScopedMutex scopedLock(m_frameHistoryLock);

	if(m_isRewinding == false || m_frameHistory.size() == 0)
	{
		//Not rewinding.  Just run the machine normally and capture its frame for the history.

		MachineFeature::RunToNextFrame();

		ScreenBuffer* screen = MachineFeature::GetStableScreenBuffer();
		if(screen != NULL)
		{
			FrameInfo frameInfo;
			frameInfo.Id = MachineFeature::GetScreenBufferCount();
			frameInfo.Screen = screen->Clone();
			m_frameHistory.push_back(frameInfo);

			while(m_frameHistory.size() > m_maxFrameHistorySize)
			{
				delete m_frameHistory.begin()->Screen;
				m_frameHistory.pop_front();
			}

			m_playbackFrame = m_frameHistory.end();
		}
	}
	else
	{
		//Rewinding.  Don't run the machine.  Just advance (backward) one step in the frame history
		if(m_playbackFrame != m_frameHistory.begin())
			m_playbackFrame--;

		printf("Rewinding to frame %d\n", m_playbackFrame->Id);
	}
}



// IEmulatedDisplay

ScreenBuffer* Rewinder::GetStableScreenBuffer()
{
	ScopedMutex scopedLock(m_frameHistoryLock);

	if(m_isRewinding == false || m_frameHistory.size() == 0 || m_playbackFrame == m_frameHistory.end())
	{
		//Not rewinding.  Just pass through.
		return MachineFeature::GetStableScreenBuffer();
	}
	else
	{
		printf("Playing history frame %d\n", m_playbackFrame->Id);

		//Rewinding.  Return our current history frame.
		return m_playbackFrame->Screen;
	}
}

int Rewinder::GetScreenBufferCount()
{
	ScopedMutex scopedLock(m_frameHistoryLock);

	if(m_isRewinding == false || m_frameHistory.size() == 0 || m_playbackFrame == m_frameHistory.end())
	{
		//Not rewinding.  Just pass through.
		return MachineFeature::GetScreenBufferCount();
	}
	else
	{
		//Rewinding.  Return our current history frame's id.
		return m_playbackFrame->Id;
	}
}
