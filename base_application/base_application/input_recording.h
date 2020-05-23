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
#ifndef INPUTRECORDING_H
#define INPUTRECORDING_H

#include <map>
#include <queue>
#include <vector>

#include "machine_feature.h"

namespace emunisce {

class Archive;

class InputRecording : public MachineFeature {
public:
	// InputRecording

	InputRecording();
	~InputRecording() override;

	void SerializeHistory(Archive& archive);
	void SerializeMovie(Archive& archive);

	void StartRecording();
	void StopRecording();

	void StartPlayback(
		bool absoluteFrames = false, bool restoreState = false,
		bool loop = false);  ///< If absoluteFrames is true, playback will only occur when the emulated machine's frame
							 ///< count matches the recorded frame count exactly.  This is only really useful for
							 ///< playback immediately after loading a savestate (rewinding).
	void StopPlayback();

	void ApplicationEvent(unsigned int eventId);
	void SetEventIdOffset(unsigned int offset);

	// MachineFeature

	void RunToNextFrame() override;

	void ButtonDown(unsigned int index) override;
	void ButtonUp(unsigned int index) override;

	bool IsButtonDown(unsigned int index) override;

private:
	bool m_recording;
	bool m_playing;

	unsigned int m_recordingStartFrame;
	unsigned int m_playbackStartFrame;

	bool m_loopPlayback;

	struct InputEvent {
		unsigned int frameId;
		unsigned int tickId;

		bool keyDown;  ///< true if a key was pressed, false if a key was released
		unsigned int keyIndex;

		void Serialize(Archive& archive);
	};

	static const unsigned int m_defaultEventIdOffset =
		0x01000000;  ///< Each application component that uses events will have a unique offset.  They're allocated
					 ///< sequentially in the high byte.
	unsigned int m_eventIdOffset;
	std::vector<InputEvent> m_inputHistory;
	unsigned char* m_startState;
	unsigned int m_startStateSize;

	std::map<unsigned int, bool> m_isButtonDown;

	std::queue<InputEvent> m_pendingEvents;
};

}  // namespace emunisce

#endif
