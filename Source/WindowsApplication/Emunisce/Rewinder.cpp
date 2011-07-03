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

#include "Emunisce.h"
#include "InputRecording.h"

#include <stdio.h>	///<printf debug


// Rewinder::Segment

Rewinder::Segment::Segment(Rewinder* rewinder, InputRecording* recorder)
{
	m_rewinder = rewinder;
	m_recorder = recorder;
}

Rewinder::Segment::~Segment()
{
}


void Rewinder::Segment::RecordFrame()
{
	//First frame
	if(m_numFramesRecorded == 0)
	{
		//Start the input recorder
	}


	//Run the machine
	m_rewinder->Internal_RunMachineToNextFrame();


	//Cache the frame?


	//Last frame
	if(m_numFramesRecorded == FramesPerSegment-1)
	{
		//Stop the input recorder and save the movie
	}
}

unsigned int Rewinder::Segment::NumFramesRecorded()
{
	return m_numFramesRecorded;
}

bool Rewinder::Segment::CanRecordMoreFrames()
{
	if(m_numFramesRecorded < FramesPerSegment)
		return true;

	return false;
}


void Rewinder::Segment::CacheFrame()
{
	//First frame
	if(m_numFramesCached == 0)
	{
		//Restore the input movie and start playing it
	}


	//Run the machine
	m_rewinder->Internal_RunMachineToNextFrame();


	//Cache the frame
}

unsigned int Rewinder::Segment::NumFramesCached()
{
	return m_numFramesCached;
}

bool Rewinder::Segment::CanCacheMoreFrames()
{
	if(m_numFramesCached < FramesPerSegment)
		return true;

	return false;
}


Rewinder::CachedFrame Rewinder::Segment::GetCachedFrame(unsigned int index)
{
	if(index < m_numFramesCached)
		return m_frameCache[index];

	Rewinder::CachedFrame default;
	return default;
}


void Rewinder::Segment::ClearCache()
{
}



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

	m_recorder = new InputRecording();
	m_recorder->SetEventIdOffset(0x02000000);	///<Use rewinder's event id offset instead of InputRecording's.  So that the application directs events to the right place.

	m_segments.push_back(new Segment(this, m_recorder));
	m_recordingSegment = 0;
	m_playingSegment = 0;
}

Rewinder::~Rewinder()
{
	m_isRewinding = false;

	while(m_frameHistory.empty() == false)
	{
		CachedFrame info = *m_frameHistory.begin();
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

void Rewinder::ApplicationEvent(unsigned int eventId)
{
	m_recorder->ApplicationEvent(eventId);
}

void Rewinder::Internal_RunMachineToNextFrame()
{
	MachineFeature::RunToNextFrame();
}


// MachineFeature

void Rewinder::SetComponentMachine(IEmulatedMachine* componentMachine)
{
	MachineFeature::SetComponentMachine(m_recorder);
	m_recorder->SetComponentMachine(componentMachine);
}

void Rewinder::SetEmulatedMachine(IEmulatedMachine* emulatedMachine)
{
	MachineFeature::SetComponentMachine(m_recorder);	///<Component, not emulated.
	m_recorder->SetEmulatedMachine(emulatedMachine);
}


// IEmulatedMachine

void Rewinder::RunToNextFrame()
{
	ScopedMutex scopedLock(m_frameHistoryLock);

	if(m_isRewinding == false || m_frameHistory.size() == 0)
	{
		//Not rewinding.  Just run the machine normally and capture its frame for the history.
		// Segment::RecordFrame runs the machine
		
		Segment* recordingSegment = NULL;
		if(m_recordingSegment < m_segments.size())
			recordingSegment = m_segments[ m_recordingSegment ];

		if(recordingSegment == NULL || recordingSegment->CanRecordMoreFrames() == false)
		{
			m_segments.push_back(new Segment(this, m_recorder));
			m_recordingSegment = m_segments.size() - 1;
			recordingSegment = m_segments[ m_recordingSegment ];
		}

		m_segments[ m_recordingSegment ]->RecordFrame();
	}
	else
	{
		//Rewinding.  Don't run the machine.  Just advance (backward) one step in the frame history.
		// Segment::CacheFrame runs the machine (playing an input movie "in the past"), generating new frames to cache.

		//If we're at the beginning of the frame history, try to get more history from the segments.
		if(m_playbackFrame == m_frameHistory.begin())
		{
			Segment* playingSegment = NULL;
			if(m_playingSegment < m_segments.size())
				playingSegment = m_segments[ m_playingSegment ];

			if(playingSegment != NULL)
			{
				//The segment should already be fully cached.  Just to be sure...
				while(playingSegment->CanCacheMoreFrames() == true)
				{
					playingSegment->CacheFrame();
				}

				//Clear the current frame history
				m_frameHistory.clear();	///<No need for deep deletion.  Segments handle that.  Screens are shallow-copied.

				for(unsigned int i=0;i<playingSegment->NumFramesCached();i++)
				{
					m_frameHistory.push_back(playingSegment->GetCachedFrame(i));
				}

				m_playbackFrame = m_frameHistory.end();
			}
			
			//Next segment
			if(m_playingSegment > 0)
				m_playingSegment--;
		}

		//If we're not at the beginning of the frame history, then advance backward one frame.
		// The frame history might have been refilled by the block above just before this, so use a bare if-check and not an else.
		if(m_playbackFrame != m_frameHistory.begin())
		{
			m_playbackFrame--;
		}

		//Advance the segment cache
		Segment* playingSegment = NULL;
		if(m_playingSegment < m_segments.size())
			playingSegment = m_segments[ m_playingSegment ];

		if(playingSegment != NULL && playingSegment->CanCacheMoreFrames())
			playingSegment->CacheFrame();

		printf("Rewinding to frame %d\n", m_playbackFrame->ScreenId);
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
		printf("Playing history frame %d\n", m_playbackFrame->ScreenId);

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
		return m_playbackFrame->ScreenId;
	}
}
