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
#include <vector>
using namespace std;


namespace Emunisce
{

class InputRecording;

class Rewinder : public MachineFeature
{
public:

	// Rewinder

	Rewinder();
	virtual ~Rewinder();

	virtual void StartRewinding();
	virtual void StopRewindRequested();

	virtual void ApplicationEvent(unsigned int eventId);

	//Used by Segment to call into MachineFeature
	virtual void Internal_RunMachineToNextFrame();
	virtual unsigned int Internal_GetFrameCount();
	virtual ScreenBuffer* Internal_GetStableScreenBuffer();
	virtual int Internal_GetScreenBufferCount();


	// MachineFeature

	virtual void SetApplication(EmunisceApplication* application);

	virtual void SetComponentMachine(IEmulatedMachine* componentMachine);	///<Overridden because this component wraps its own InputRecording (m_recorder)
	virtual void SetEmulatedMachine(IEmulatedMachine* emulatedMachine);	///<Overridden because this component wraps its own InputRecording (m_recorder)


	// IEmulatedMachine

	virtual void RunToNextFrame();


	// IEmulatedDisplay

	virtual ScreenBuffer* GetStableScreenBuffer();
	virtual int GetScreenBufferCount();


protected:

	virtual void StopRewinding();

	bool m_isRewinding;
	bool m_stopRewindRequested;
	InputRecording* m_recorder;	///<Private recorder so it doesn't conflict with user movies/macros.

	struct CachedFrame
	{
		unsigned int MachineFrameId;	///<From Machine::GetFrameCount

		unsigned int ScreenId;	///<From IEmulatedDisplay::GetScreenBufferCount
		ScreenBuffer* Screen;

		CachedFrame()
		{
			MachineFrameId = (unsigned int)-1;
			ScreenId = (unsigned int)-1;
			Screen = NULL;
		}
	};

	class Segment
	{
	public:

		static const unsigned int FramesPerSegment = 60;

		Segment(Rewinder* rewinder, InputRecording* recorder);
		~Segment();

		void RecordFrame();
		unsigned int NumFramesRecorded();	///<The number of frames currently recorded by this segment
		bool CanRecordMoreFrames();

		void CacheFrame();
		unsigned int NumFramesCached();	///<The number of frames cached and ready for playback
		bool CanCacheMoreFrames();

		CachedFrame GetCachedFrame(unsigned int index);

		void ClearCache();

		void RestoreState();


	private:

		Rewinder* m_rewinder;
		InputRecording* m_recorder;

		unsigned char* m_inputMovieData;
		unsigned int m_inputMovieDataSize;

		unsigned int m_numFramesRecorded;

		CachedFrame m_frameCache[FramesPerSegment];
		unsigned int m_numFramesCached;
	};

	static const unsigned int m_maxSegments = 60;	///<Keep 60 seconds of rewind history
	vector<Segment*> m_segments;
	unsigned int m_recordingSegment;
	unsigned int m_playingSegment;

	list<CachedFrame> m_frameHistory;
	list<CachedFrame>::iterator m_playbackFrame;
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

		virtual bool IsButtonDown(unsigned int index);


	private:

		Rewinder* m_rewinder;
	};

	InputHandler* m_inputHandler;
};

}	//namespace Emunisce

#endif
