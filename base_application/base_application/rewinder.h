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

#include <list>
#include <mutex>
#include <vector>

#include "machine_feature.h"
#include "platform_includes.h"

namespace emunisce {

class InputRecording;

class Rewinder : public MachineFeature {
public:
	// Rewinder

	Rewinder();
	~Rewinder() override;

	virtual void StartRewinding();
	virtual void StopRewinding();

	virtual void ApplicationEvent(unsigned int eventId);

	// Used by Segment to call into MachineFeature
	virtual void Internal_RunMachineToNextFrame();
	virtual unsigned int Internal_GetFrameCount();
	virtual ScreenBuffer* Internal_GetStableScreenBuffer();
	virtual int Internal_GetScreenBufferCount();
	virtual AudioBuffer Internal_GetStableAudioBuffer();
	virtual int Internal_GetAudioBufferCount();

	// MachineFeature

	void SetApplication(BaseApplication* application) override;

	void SetComponentMachine(EmulatedMachine* componentMachine)
		override;  ///< Overridden because this component wraps its own InputRecording (m_recorder)
	void SetEmulatedMachine(EmulatedMachine* emulatedMachine)
		override;  ///< Overridden because this component wraps its own InputRecording (m_recorder)

	// EmulatedMachine

	unsigned int GetFrameCount() override;
	void RunToNextFrame() override;

	// IEmulatedDisplay

	ScreenBuffer* GetStableScreenBuffer() override;
	int GetScreenBufferCount() override;

	// IEmulatedSound

	AudioBuffer GetStableAudioBuffer() override;
	int GetAudioBufferCount() override;

protected:
	bool m_isRewinding;
	InputRecording* m_recorder;  ///< Private recorder so it doesn't conflict with user movies/macros.

	struct CachedFrame {
		unsigned int MachineFrameId;  ///< From Machine::GetFrameCount

		unsigned int ScreenBufferId;  ///< From IEmulatedDisplay::GetScreenBufferCount
		ScreenBuffer* Screen;

		unsigned int AudioBufferId;
		AudioBuffer Audio;

		CachedFrame() {
			MachineFrameId = (unsigned int)-1;

			ScreenBufferId = (unsigned int)-1;
			Screen = nullptr;

			AudioBufferId = (unsigned int)-1;
		}
	};

	class Segment {
	public:
		static const unsigned int FramesPerSegment = 60;

		Segment(Rewinder* rewinder, InputRecording* recorder);
		~Segment();

		void RecordFrame();
		unsigned int NumFramesRecorded();  ///< The number of frames currently recorded by this segment
		bool CanRecordMoreFrames();

		void CacheFrame();
		unsigned int NumFramesCached();  ///< The number of frames cached and ready for playback
		bool CanCacheMoreFrames();

		CachedFrame GetCachedFrame(unsigned int index);

		void ClearCache();

		void RestoreState();

		void LockAtFrame(
			unsigned int frameId);  ///< Disables all history within this segment newer than the specified frame.

	private:
		Rewinder* m_rewinder;
		InputRecording* m_recorder;

		unsigned char* m_inputMovieData;
		unsigned int m_inputMovieDataSize;

		unsigned int m_numFramesRecorded;

		CachedFrame m_frameCache[FramesPerSegment];
		unsigned int m_numFramesCached;

		bool m_locked;
	};

	static const unsigned int m_maxSegments = 60;  ///< Keep 60 seconds of rewind history
	std::vector<Segment*> m_segments;
	unsigned int m_playingSegment;

	std::list<CachedFrame> m_frameHistory;
	std::list<CachedFrame>::iterator m_playbackFrame;
	static const unsigned int m_maxFrameHistorySize = Segment::FramesPerSegment;
	std::mutex m_frameHistoryLock;

	class InputHandler : public EmulatedInput {
	public:
		InputHandler(Rewinder* rewinder);
		virtual ~InputHandler() = default;

		// IEmulatedInput

		unsigned int NumButtons() override;
		const char* GetButtonName(unsigned int index) override;

		void ButtonDown(unsigned int index) override;
		void ButtonUp(unsigned int index) override;

		bool IsButtonDown(unsigned int index) override;

	private:
		Rewinder* m_rewinder;
	};

	InputHandler* m_inputHandler;
};

}  // namespace emunisce

#endif
