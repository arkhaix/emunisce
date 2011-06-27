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

#include "Emunisce.h"
#include "MachineRunner.h"

#include "Serialization/SerializationIncludes.h"


// InputEvent

void InputRecording::InputEvent::Serialize(Archive& archive)
{
	SerializeItem(archive, frameId);
	SerializeItem(archive, tickId);

	SerializeItem(archive, keyDown);
	SerializeItem(archive, keyIndex);
}


// InputRecording

InputRecording::InputRecording()
{
	m_hasFocus = false;

	m_recording = false;
	m_playing = false;

	m_recordingStartFrame = 0;
}


void InputRecording::SerializeMovie(Archive& archive)
{
	SerializeItem(archive, m_recordingStartFrame);

	unsigned int numEvents = m_movie.size();
	SerializeItem(archive, numEvents);

	if(archive.GetArchiveMode() == ArchiveMode::Loading)
	{
		m_movie.clear();

		for(int i=0;i<numEvents;i++)
		{
			InputEvent inputEvent;
			inputEvent.Serialize(archive);

			m_movie.push_back(inputEvent);
		}
	}
	else //archive.GetArchiveMode() == ArchiveMode::Saving
	{
		for(auto iter = m_movie.begin(); iter != m_movie.end(); ++iter)
		{
			InputEvent& inputEvent = *iter;
			inputEvent.Serialize(archive);
		}
	}
}


void InputRecording::StartRecording()
{
	if(m_wrappedMachine != NULL)
	{
		bool wasPaused = m_application->GetMachineRunner()->IsPaused();
		m_application->GetMachineRunner()->Pause();

		m_movie.clear();

		m_recording = true;
		m_recordingStartFrame = m_wrappedMachine->GetFrameCount();

		if(wasPaused == false)
			m_application->GetMachineRunner()->Run();
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
		bool wasPaused = m_application->GetMachineRunner()->IsPaused();
		m_application->GetMachineRunner()->Pause();

		m_playing = true;

		for(unsigned int i=0;i<m_movie.size();i++)
		{
			Emunisce::ApplicationEvent inputEvent;
			inputEvent.eventId = m_eventIdOffset + i;
			inputEvent.frameCount = m_movie[i].frameId;
			inputEvent.tickCount = m_movie[i].tickId;

			if(absoluteFrames == false)
				inputEvent.frameCount -= m_recordingStartFrame;

			m_wrappedMachine->AddApplicationEvent(inputEvent, !absoluteFrames);
		}

		if(wasPaused == false)
			m_application->GetMachineRunner()->Run();
	}
}

void InputRecording::StopPlayback()
{
	m_playing = false;
}


void InputRecording::ApplicationEvent(unsigned int eventId)
{
	if(m_wrappedInput == NULL)
		return;

	if(m_playing == false)
		return;

	eventId -= m_eventIdOffset;
	if(eventId < m_movie.size())
	{
		InputEvent& inputEvent = m_movie[eventId];
		if(inputEvent.keyDown == true)
		{
			m_wrappedInput->ButtonDown(inputEvent.keyIndex);
		}
		else //inputEvent.keyDown == false (keyUp)
		{
			m_wrappedInput->ButtonUp(inputEvent.keyIndex);
		}
	}
}



// MachineFeature

#include <stdio.h>
void InputRecording::ButtonDown(unsigned int index)
{
	auto iter = m_isButtonDown.find(index);
	if(iter != m_isButtonDown.end() && iter->second == true)
		return;

	m_isButtonDown[index] = true;

	if(m_recording == true && m_wrappedMachine != NULL)
	{
		printf("InputRecording::ButtonDown(%d)\n", index);

		InputEvent inputEvent;
		inputEvent.frameId = m_wrappedMachine->GetFrameCount();
		inputEvent.tickId = m_wrappedMachine->GetTickCount();
		inputEvent.keyDown = true;
		inputEvent.keyIndex = index;

		m_movie.push_back(inputEvent);
	}

	MachineFeature::ButtonDown(index);
}

void InputRecording::ButtonUp(unsigned int index)
{
	auto iter = m_isButtonDown.find(index);
	if(iter != m_isButtonDown.end() && iter->second == false)
		return;

	m_isButtonDown[index] = false;

	if(m_recording == true && m_wrappedMachine != NULL)
	{
		printf("InputRecording::ButtonUp(%d)\n", index);

		InputEvent inputEvent;
		inputEvent.frameId = m_wrappedMachine->GetFrameCount();
		inputEvent.tickId = m_wrappedMachine->GetTickCount();
		inputEvent.keyDown = false;
		inputEvent.keyIndex = index;

		m_movie.push_back(inputEvent);
	}

	MachineFeature::ButtonUp(index);
}
