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
#include "InputRecording.h"
using namespace Emunisce;

#include "Serialization/SerializationIncludes.h"


// InputRecording

InputRecording::InputRecording()
{
	m_hasFocus = false;

	m_recording = false;
	m_playing = false;

	m_recordingStartFrame = 0;
	m_playbackStartFrame = 0;
	m_absoluteFramePlayback = false;
}


void InputRecording::SerializeMovie(Archive& archive)
{
	unsigned int numEvents = m_movie.size();
	SerializeItem(archive, numEvents);

	if(archive.GetArchiveMode() == ArchiveMode::Loading)
	{
		m_movie.clear();

		for(int i=0;i<numEvents;i++)
		{
			unsigned int frame;
			SerializeItem(archive, frame);

			InputEvent inputEvent;
			SerializeItem(archive, inputEvent.keyDown);
			SerializeItem(archive, inputEvent.keyIndex);

			m_movie.insert( make_pair(frame, inputEvent) );
		}
	}
	else //archive.GetArchiveMode() == ArchiveMode::Saving
	{
		for(auto iter = m_movie.begin(); iter != m_movie.end(); ++iter)
		{
			unsigned int frame = iter->first;
			SerializeItem(archive, frame);

			InputEvent& inputEvent = iter->second;
			SerializeItem(archive, inputEvent.keyDown);
			SerializeItem(archive, inputEvent.keyIndex);
		}
	}
}


void InputRecording::StartRecording()
{
	if(m_wrappedMachine != NULL)
	{
		m_recording = true;
		m_recordingStartFrame = m_wrappedMachine->GetFrameCount();
	}
}

void InputRecording::StopRecording()
{
	m_recording = false;
}


void InputRecording::StartPlayback(bool absoluteFrames)
{
	if(m_wrappedMachine != NULL)
	{
		m_playing = true;
		m_playbackStartFrame = m_wrappedMachine->GetFrameCount();
		m_absoluteFramePlayback = absoluteFrames;
	}
}

void InputRecording::StopPlayback()
{
	m_playing = false;
}



// MachineFeature

void InputRecording::RunToNextFrame()
{
	if(m_playing == true && m_wrappedMachine != NULL && m_wrappedInput != NULL)
	{
		unsigned int currentFrame = m_wrappedMachine->GetFrameCount();

		int playbackFrameDelta = m_recordingStartFrame - m_playbackStartFrame;
		if(m_absoluteFramePlayback == false)
			currentFrame += playbackFrameDelta;

		auto iterRange = m_movie.equal_range(currentFrame);
		for(auto iter = iterRange.first; iter != iterRange.second; iter++)
		{
			InputEvent& inputEvent = iter->second;
			if(inputEvent.keyDown == true)
			{
				m_wrappedInput->ButtonDown(inputEvent.keyIndex);
			}
			else //inputEvent.keyDown == false
			{
				m_wrappedInput->ButtonUp(inputEvent.keyIndex);
			}
		}
	}

	MachineFeature::RunToNextFrame();
}


void InputRecording::ButtonDown(unsigned int index)
{
	if(m_recording == true && m_wrappedMachine != NULL)
	{
		unsigned int currentFrame = m_wrappedMachine->GetFrameCount();

		InputEvent inputEvent;
		inputEvent.keyDown = true;
		inputEvent.keyIndex = index;

		m_movie.insert( make_pair(currentFrame, inputEvent) );
	}

	MachineFeature::ButtonDown(index);
}

void InputRecording::ButtonUp(unsigned int index)
{
	if(m_recording == true && m_wrappedMachine != NULL)
	{
		unsigned int currentFrame = m_wrappedMachine->GetFrameCount();

		InputEvent inputEvent;
		inputEvent.keyDown = false;
		inputEvent.keyIndex = index;

		m_movie.insert( make_pair(currentFrame, inputEvent) );
	}

	MachineFeature::ButtonUp(index);
}
