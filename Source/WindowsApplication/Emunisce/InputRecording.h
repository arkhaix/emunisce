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

#include "MachineFeature.h"

#include <map>
#include <vector>
using namespace std;


namespace Emunisce
{

class Archive;

class InputRecording : public MachineFeature
{
public:

	// InputRecording

	InputRecording();
	
	void SerializeMovie(Archive& archive);

	void StartRecording();
	void StopRecording();

	void StartPlayback(bool absoluteFrames = false);	///<If absoluteFrames is true, playback will only occur when the emulated machine's frame count matches the recorded frame count exactly.  This is only really useful for playback immediately after loading a savestate (rewinding).
	void StopPlayback();

	void ApplicationEvent(unsigned int eventId);


	// MachineFeature

	virtual void ButtonDown(unsigned int index);
	virtual void ButtonUp(unsigned int index);

private:

	bool m_recording;
	bool m_playing;

	unsigned int m_recordingStartFrame;

	struct InputEvent
	{
		unsigned int frameId;
		unsigned int tickId;

		bool keyDown;			///<true if a key was pressed, false if a key was released
		unsigned int keyIndex;

		void Serialize(Archive& archive);
	};

	static const unsigned int m_eventIdOffset = 0x01000000;	///<Each application component that uses events will have a unique offset.  They're allocated sequentially in the high byte.
	vector<InputEvent> m_movie;

	map<unsigned int, bool> m_isButtonDown;
};

}	//namespace Emunisce

#endif
