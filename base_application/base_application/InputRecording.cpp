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
using namespace emunisce;

#include "BaseApplication.h"
#include "MachineRunner.h"
#include "serialization/MemorySerializer.h"
#include "serialization/SerializationIncludes.h"

// InputEvent

void InputRecording::InputEvent::Serialize(Archive& archive) {
	SerializeItem(archive, frameId);
	SerializeItem(archive, tickId);

	SerializeItem(archive, keyDown);
	SerializeItem(archive, keyIndex);
}

// InputRecording

InputRecording::InputRecording() {
	m_hasFocus = false;

	m_recording = false;
	m_playing = false;

	m_recordingStartFrame = 0;
	m_playbackStartFrame = 0;

	m_loopPlayback = false;

	m_startState = nullptr;
	m_startStateSize = 0;

	m_eventIdOffset = m_defaultEventIdOffset;
}

InputRecording::~InputRecording() {
	if (m_startState != nullptr) {
		delete m_startState;
	}
}

void InputRecording::SerializeHistory(Archive& archive) {
	SerializeItem(archive, m_recordingStartFrame);

	unsigned int numEvents = (unsigned int)m_inputHistory.size();
	SerializeItem(archive, numEvents);

	if (archive.GetArchiveMode() == ArchiveMode::Loading) {
		m_inputHistory.clear();

		for (unsigned int i = 0; i < numEvents; i++) {
			InputEvent inputEvent;
			inputEvent.Serialize(archive);

			m_inputHistory.push_back(inputEvent);
		}
	}
	else  // archive.GetArchiveMode() == ArchiveMode::Saving
	{
		for (auto& inputEvent : m_inputHistory) {
			inputEvent.Serialize(archive);
		}
	}
}

void InputRecording::SerializeMovie(Archive& archive) {
	SerializeItem(archive, m_startStateSize);
	if (archive.GetArchiveMode() == ArchiveMode::Loading) {
		if (m_startState) {
			delete m_startState;
		}

		m_startState = (unsigned char*)malloc(m_startStateSize);
	}

	SerializeBuffer(archive, m_startState, m_startStateSize);

	SerializeHistory(archive);
}

void InputRecording::StartRecording() {
	if (m_wrappedMachine != nullptr) {
		// Pause the machine
		bool wasPaused = m_application->GetMachineRunner()->IsPaused();
		m_application->GetMachineRunner()->Pause();

		// Record the initial machine state
		m_inputHistory.clear();

		m_recording = true;
		m_recordingStartFrame = m_wrappedMachine->GetFrameCount();

		MemorySerializer serializer;
		Archive archive(&serializer, ArchiveMode::Saving);
		m_wrappedMachine->SaveState(archive);

		if (m_startState != nullptr) {
			delete m_startState;
		}

		serializer.TransferBuffer(&m_startState, &m_startStateSize);

		// Resume the machine
		if (wasPaused == false) {
			m_application->GetMachineRunner()->Run();
		}
	}
}

void InputRecording::StopRecording() {
	m_recording = false;
}

void InputRecording::StartPlayback(bool absoluteFrames, bool restoreState, bool loop) {
	if (m_wrappedMachine != nullptr) {
		// Pause the machine
		bool wasPaused = m_application->GetMachineRunner()->IsPaused();
		m_application->GetMachineRunner()->Pause();

		m_playbackStartFrame = m_wrappedMachine->GetFrameCount();

		m_loopPlayback = loop;

		m_playing = true;

		// Restore the initial savestate
		if (restoreState == true) {
			MemorySerializer serializer;
			serializer.SetBuffer(m_startState, m_startStateSize);
			Archive archive(&serializer, ArchiveMode::Loading);
			m_wrappedMachine->LoadState(archive);
		}

		// Add all the recorded events
		for (unsigned int i = 0; i < m_inputHistory.size(); i++) {
			emunisce::ApplicationEvent inputEvent;
			inputEvent.eventId = m_eventIdOffset + i;
			inputEvent.frameCount = m_inputHistory[i].frameId;
			inputEvent.tickCount = m_inputHistory[i].tickId;

			if (absoluteFrames == false) {
				inputEvent.frameCount -= m_recordingStartFrame;
			}

			m_wrappedMachine->AddApplicationEvent(inputEvent, !absoluteFrames);
		}

		// Resume the machine
		if (wasPaused == false) {
			m_application->GetMachineRunner()->Run();
		}
	}
}

void InputRecording::StopPlayback() {
	m_playing = false;

	if (m_wrappedMachine != nullptr) {
		for (unsigned int i = 0; i < m_inputHistory.size(); i++) {
			m_wrappedMachine->RemoveApplicationEvent(m_eventIdOffset + i);
		}
	}
}

void InputRecording::ApplicationEvent(unsigned int eventId) {
	if (m_wrappedInput == nullptr) {
		return;
	}

	if (m_playing == false) {
		return;
	}

	eventId -= m_eventIdOffset;
	if (eventId < m_inputHistory.size()) {
		InputEvent& inputEvent = m_inputHistory[eventId];
		if (inputEvent.keyDown == true) {
			m_wrappedInput->ButtonDown(inputEvent.keyIndex);
		}
		else  // inputEvent.keyDown == false (keyUp)
		{
			m_wrappedInput->ButtonUp(inputEvent.keyIndex);
		}

		if (eventId == m_inputHistory.size() - 1 && m_playing == true && m_loopPlayback == true) {
			StartPlayback(false, false, true);
		}
	}
}

void InputRecording::SetEventIdOffset(unsigned int offset) {
	m_eventIdOffset = offset;
}

// MachineFeature

void InputRecording::RunToNextFrame() {
	if (m_recording == true && m_wrappedMachine != nullptr) {
		while (m_pendingEvents.empty() == false) {
			InputEvent& inputEvent = m_pendingEvents.front();

			if (inputEvent.keyDown == true) {
				MachineFeature::ButtonDown(inputEvent.keyIndex);
			}
			else {
				MachineFeature::ButtonUp(inputEvent.keyIndex);
			}

			inputEvent.frameId = m_wrappedMachine->GetFrameCount();
			inputEvent.tickId = m_wrappedMachine->GetTickCount();

			m_inputHistory.push_back(inputEvent);

			m_pendingEvents.pop();
		}
	}

	MachineFeature::RunToNextFrame();
}

void InputRecording::ButtonDown(unsigned int index) {
	auto iter = m_isButtonDown.find(index);
	if (iter != m_isButtonDown.end() && iter->second == true) {
		return;
	}

	m_isButtonDown[index] = true;

	if (m_recording == true && m_wrappedMachine != nullptr) {
		InputEvent inputEvent;

		inputEvent.keyDown = true;
		inputEvent.keyIndex = index;

		m_pendingEvents.push(inputEvent);
	}
	else {
		MachineFeature::ButtonDown(index);
	}
}

void InputRecording::ButtonUp(unsigned int index) {
	auto iter = m_isButtonDown.find(index);
	if (iter != m_isButtonDown.end() && iter->second == false) {
		return;
	}

	m_isButtonDown[index] = false;

	if (m_recording == true && m_wrappedMachine != nullptr) {
		InputEvent inputEvent;

		inputEvent.keyDown = false;
		inputEvent.keyIndex = index;

		m_pendingEvents.push(inputEvent);
	}
	else {
		MachineFeature::ButtonUp(index);
	}
}

bool InputRecording::IsButtonDown(unsigned int index) {
	auto iter = m_isButtonDown.find(index);
	if (iter == m_isButtonDown.end()) {
		return false;
	}

	if (iter->second == false) {
		return false;
	}

	return true;
}
