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

#include "BaseApplication.h"
#include "MachineRunner.h"

#include "InputRecording.h"

#include "Serialization/SerializationIncludes.h"
#include "Serialization/MemorySerializer.h"


// Rewinder::Segment

Rewinder::Segment::Segment(Rewinder* rewinder, InputRecording* recorder)
{
	m_rewinder = rewinder;
	m_recorder = recorder;

	m_inputMovieData = nullptr;
	m_inputMovieDataSize = 0;

	m_numFramesRecorded = 0;
	m_numFramesCached = 0;

	m_locked = false;
}

Rewinder::Segment::~Segment()
{
	ClearCache();

	if (m_inputMovieData != nullptr) {
		delete m_inputMovieData;
	}
}


void Rewinder::Segment::RecordFrame()
{
	if (m_locked == true) {
		return;
	}


	//First frame
	if (m_numFramesRecorded == 0)
	{
		//Start the input recorder
		m_recorder->StartRecording();
	}


	//Run the machine
	m_rewinder->Internal_RunMachineToNextFrame();


	//Cache the frame
	CachedFrame& frame = m_frameCache[m_numFramesRecorded];
	frame.MachineFrameId = m_rewinder->Internal_GetFrameCount();
	frame.ScreenBufferId = m_rewinder->Internal_GetScreenBufferCount();
	frame.Screen = m_rewinder->Internal_GetStableScreenBuffer()->Clone();


	//Last frame
	if (m_numFramesRecorded == FramesPerSegment - 1)
	{
		//Stop the input recorder and save the movie
		m_recorder->StopRecording();

		MemorySerializer serializer;
		Archive archive(&serializer, ArchiveMode::Saving);

		m_recorder->SerializeMovie(archive);

		serializer.TransferBuffer(&m_inputMovieData, &m_inputMovieDataSize);
	}

	m_numFramesRecorded++;
	m_numFramesCached = m_numFramesRecorded;
}

unsigned int Rewinder::Segment::NumFramesRecorded()
{
	return m_numFramesRecorded;
}

bool Rewinder::Segment::CanRecordMoreFrames()
{
	if (m_locked == true) {
		return false;
	}

	if (m_numFramesRecorded < FramesPerSegment) {
		return true;
	}

	return false;
}


void Rewinder::Segment::CacheFrame()
{
	//First frame
	if (m_numFramesCached == 0)
	{
		//Stop recording
		m_recorder->StopRecording();

		//Restore the input movie and start playing it
		RestoreState();

		//If there's no movie data in this segment, it means we're the most recent segment
		// so the recorder was still recording our movie until CacheFrame was called.

		m_recorder->StartPlayback(false, true, false);
	}


	//Run the machine
	m_rewinder->Internal_RunMachineToNextFrame();


	//Cache the frame
	if (m_numFramesCached < FramesPerSegment)
	{
		CachedFrame& frame = m_frameCache[m_numFramesCached];
		frame.MachineFrameId = m_rewinder->Internal_GetFrameCount();
		frame.ScreenBufferId = m_rewinder->Internal_GetScreenBufferCount();
		frame.Screen = m_rewinder->Internal_GetStableScreenBuffer()->Clone();
		frame.AudioBufferId = m_rewinder->Internal_GetAudioBufferCount();
		frame.Audio = m_rewinder->Internal_GetStableAudioBuffer();

		//Reverse the audio
		for (unsigned int i = 0; i < frame.Audio.NumSamples / 2; i++)
		{
			std::swap(frame.Audio.Samples[0][i], frame.Audio.Samples[0][frame.Audio.NumSamples - i - 1]);
			std::swap(frame.Audio.Samples[1][i], frame.Audio.Samples[1][frame.Audio.NumSamples - i - 1]);
		}

		m_numFramesCached++;
	}
}

unsigned int Rewinder::Segment::NumFramesCached()
{
	return m_numFramesCached;
}

bool Rewinder::Segment::CanCacheMoreFrames()
{
	if (m_numFramesCached < FramesPerSegment && m_numFramesCached < m_numFramesRecorded) {
		return true;
	}

	return false;
}


Rewinder::CachedFrame Rewinder::Segment::GetCachedFrame(unsigned int index)
{
	if (index < m_numFramesCached) {
		return m_frameCache[index];
	}

	Rewinder::CachedFrame defaultFrame;
	return defaultFrame;
}


void Rewinder::Segment::ClearCache()
{
	CachedFrame defaultFrame;

	for (unsigned int i = 0; i < m_numFramesCached; i++)
	{
		delete m_frameCache[i].Screen;
		m_frameCache[i] = defaultFrame;
	}

	m_numFramesCached = 0;
}


void Rewinder::Segment::RestoreState()
{
	if (m_inputMovieData != nullptr && m_inputMovieDataSize > 0)
	{
		MemorySerializer serializer;
		serializer.SetBuffer(m_inputMovieData, m_inputMovieDataSize);
		Archive archive(&serializer, ArchiveMode::Loading);

		m_recorder->SerializeMovie(archive);
	}
}


void Rewinder::Segment::LockAtFrame(unsigned int frameId)
{
	for (unsigned int i = 0; i < m_numFramesRecorded; i++)
	{
		if (m_frameCache[i].MachineFrameId == frameId)
		{
			//Found the specified frame.  Kill everything after this one.

			CachedFrame defaultFrame;
			for (unsigned int j = i + 1; j < m_numFramesRecorded; j++)
			{
				if (m_frameCache[j].Screen != nullptr) {
					delete m_frameCache[j].Screen;
				}

				m_frameCache[j] = defaultFrame;
			}

			m_locked = true;
			m_numFramesRecorded = i + 1;
			m_numFramesCached = m_numFramesRecorded;
			break;
		}
	}
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
	if (index >= RewinderButtons::NumButtons) {
		return nullptr;
	}

	return RewinderButtons::ToString[index];
}


void Rewinder::InputHandler::ButtonDown(unsigned int index)
{
	if (index == RewinderButtons::Rewind) {
		m_rewinder->StartRewinding();
	}
}

void Rewinder::InputHandler::ButtonUp(unsigned int index)
{
	if (index == RewinderButtons::Rewind) {
		m_rewinder->StopRewinding();
	}
}

bool Rewinder::InputHandler::IsButtonDown(unsigned int /*index*/)
{
	return false;
}



// Rewinder

Rewinder::Rewinder()
{
	m_inputHandler = new Rewinder::InputHandler(this);
	m_featureInput = m_inputHandler;

	m_playbackFrame = m_frameHistory.end();

	m_isRewinding = false;

	m_recorder = new InputRecording();
	m_recorder->SetEventIdOffset(0x02000000);	///<Use rewinder's event id offset instead of InputRecording's so that the application directs events to the right place.

	m_playingSegment = 0;
}

Rewinder::~Rewinder()
{
	m_isRewinding = false;

	for (auto& segment : m_segments)
	{
		delete segment;
	}

	m_segments.clear();
	m_frameHistory.clear();	///<Don't need to delete anything here because the screens are shallow copied from the segments

	m_featureInput = nullptr;
	delete m_inputHandler;
}


void Rewinder::StartRewinding()
{
	if (m_segments.empty()) {
		return;
	}

	bool wasPaused = m_application->GetMachineRunner()->IsPaused();
	m_application->GetMachineRunner()->Pause();

	{
		std::lock_guard<std::mutex> scopedLock(m_frameHistoryLock);

		m_playingSegment = (unsigned int)m_segments.size() - 1;

		Segment* playingSegment = m_segments[m_playingSegment];

		while (playingSegment->CanCacheMoreFrames()) {
			playingSegment->CacheFrame();
		}

		m_frameHistory.clear();
		for (unsigned int i = 0; i < playingSegment->NumFramesCached(); i++) {
			m_frameHistory.push_back(playingSegment->GetCachedFrame(i));
		}

		if (m_playingSegment > 0) {
			m_playingSegment--;
		}

		m_isRewinding = true;
		m_playbackFrame = m_frameHistory.end();
		--m_playbackFrame;
	}

	if (wasPaused == false) {
		m_application->GetMachineRunner()->Run();
	}
}

void Rewinder::StopRewinding()
{
	bool wasPaused = m_application->GetMachineRunner()->IsPaused();
	m_application->GetMachineRunner()->Pause();

	unsigned int visibleSegmentIndex = m_playingSegment + 1;
	Segment* visibleSegment = nullptr;

	{
		std::lock_guard<std::mutex> scopedLock(m_frameHistoryLock);

		visibleSegmentIndex = m_playingSegment + 1;
		visibleSegment = nullptr;

		m_isRewinding = false;

		//m_playingSegment represents the segment being played in the background to generate caches.
		// However, the currently visible segment is m_playingSegment+1 -- it's the segment that most
		// recently finished generating caches and whose screen buffers are being displayed.
		if (visibleSegmentIndex >= m_segments.size()) {
			visibleSegmentIndex = (unsigned int)m_segments.size() - 1;
		}

		if (visibleSegmentIndex < m_segments.size()) {
			visibleSegment = m_segments[visibleSegmentIndex];
		}
	}


	//Run the visible segment up until we hit the frame that's currently being displayed

	if (visibleSegment != nullptr)
	{
		visibleSegment->ClearCache();

		visibleSegment->RestoreState();

		while (visibleSegment->CanCacheMoreFrames() && m_wrappedMachine->GetFrameCount() != m_playbackFrame->MachineFrameId) {
			visibleSegment->CacheFrame();
		}

		visibleSegment->ClearCache();

		visibleSegment->LockAtFrame(m_playbackFrame->MachineFrameId);
	}


	{
		std::lock_guard<std::mutex> scopedLock(m_frameHistoryLock);

		//Delete all the segments newer than the visible one.  The old future is gone.

		unsigned int oldFutureSegmentIndex = visibleSegmentIndex + 1;

		if (m_segments.empty() == false && oldFutureSegmentIndex < m_segments.size())
		{
			for (unsigned int i = oldFutureSegmentIndex; i < m_segments.size(); i++) {
				delete m_segments[i];
			}

			m_segments.erase(m_segments.begin() + oldFutureSegmentIndex, m_segments.end());

			m_segments.push_back(new Segment(this, m_recorder));
			m_playingSegment = (unsigned int)m_segments.size() - 1;
		}


		//Clear input state so keys don't get stuck
		if (m_wrappedInput != nullptr)
		{
			for (unsigned int i = 0; i < m_wrappedInput->NumButtons(); i++)
			{
				m_wrappedInput->ButtonDown(i);
				m_wrappedInput->ButtonUp(i);
			}
		}
	}


	if (wasPaused == false) {
		m_application->GetMachineRunner()->Run();
	}
}

void Rewinder::ApplicationEvent(unsigned int eventId)
{
	m_recorder->ApplicationEvent(eventId);
}

void Rewinder::Internal_RunMachineToNextFrame()
{
	MachineFeature::RunToNextFrame();
}

unsigned int Rewinder::Internal_GetFrameCount()
{
	return MachineFeature::GetFrameCount();
}

ScreenBuffer* Rewinder::Internal_GetStableScreenBuffer()
{
	return MachineFeature::GetStableScreenBuffer();
}

int Rewinder::Internal_GetScreenBufferCount()
{
	return MachineFeature::GetScreenBufferCount();
}

AudioBuffer Rewinder::Internal_GetStableAudioBuffer()
{
	return MachineFeature::GetStableAudioBuffer();
}

int Rewinder::Internal_GetAudioBufferCount()
{
	return MachineFeature::GetAudioBufferCount();
}



// MachineFeature

void Rewinder::SetApplication(BaseApplication* application)
{
	MachineFeature::SetApplication(application);
	m_recorder->SetApplication(application);
}

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

unsigned int Rewinder::GetFrameCount()
{
	std::lock_guard<std::mutex> scopedLock(m_frameHistoryLock);

	if (m_isRewinding == false || m_frameHistory.empty())
	{
		//Not rewinding.  Just pass through.
		return MachineFeature::GetFrameCount();
	}

	//Rewinding.  Return the frame id of the current frame in the history.
	return m_playbackFrame->MachineFrameId;
}

void Rewinder::RunToNextFrame()
{
	std::lock_guard<std::mutex> scopedLock(m_frameHistoryLock);

	if (m_isRewinding == false || m_frameHistory.empty())
	{
		//Not rewinding.  Just run the machine normally and capture its frame for the history.
		// Segment::RecordFrame runs the machine

		unsigned int lastSegmentIndex = (unsigned int)m_segments.size() - 1;

		Segment* recordingSegment = nullptr;
		if (lastSegmentIndex < m_segments.size()) {	///<This check is here in case m_segments.size() == 0
			recordingSegment = m_segments[lastSegmentIndex];
		}

		if (recordingSegment == nullptr || recordingSegment->CanRecordMoreFrames() == false)
		{
			m_segments.push_back(new Segment(this, m_recorder));
			lastSegmentIndex = (unsigned int)m_segments.size() - 1;
			recordingSegment = m_segments[lastSegmentIndex];

			//Clear old caches
			for (auto& segment : m_segments) {
				segment->ClearCache();
			}

			//Delete earliest segments if we have too many
			while (m_segments.size() >= m_maxSegments)
			{
				delete m_segments[0];
				m_segments.erase(m_segments.begin());
				lastSegmentIndex = (unsigned int)m_segments.size() - 1;
			}
		}

		m_segments[lastSegmentIndex]->RecordFrame();
	}
	else
	{
		//Rewinding.  Advance (backward) one step in the frame history.
		// Segment::CacheFrame runs the machine (playing an input movie "in the past"), generating new frames to cache.

		//If we're at the beginning of the frame history, try to get more history from the segments.
		if (m_playbackFrame == m_frameHistory.begin())
		{
			Segment* playingSegment = nullptr;
			if (m_playingSegment < m_segments.size()) {
				playingSegment = m_segments[m_playingSegment];
			}

			if (playingSegment != nullptr)
			{
				//The segment should already be fully cached (except the first segment when rewinding has just begun).
				while (playingSegment->CanCacheMoreFrames() == true) {
					playingSegment->CacheFrame();
				}

				//Replace the active frame history with the segment data
				m_frameHistory.clear();

				for (unsigned int i = 0; i < playingSegment->NumFramesCached(); i++)
				{
					//Note: This is very hackish and is currently Gameboy-specific.
					// The first frame after restoring state contains garbage.  This is very visible
					// when restoring lots of states frequently and expecting them to blend together nicely.
					// To work around this, I'm doubling up the second frame and copying it over the first.
					// <--
					if (i == 0 && playingSegment->NumFramesCached() > 1) {
						continue;
					}

					if (i == 1) {
						m_frameHistory.push_back(playingSegment->GetCachedFrame(i));
					}
					// -->

					m_frameHistory.push_back(playingSegment->GetCachedFrame(i));
				}

				m_playbackFrame = m_frameHistory.end();
			}

			//Next segment
			if (m_playingSegment > 0) {
				m_playingSegment--;
			}
		}

		//If we're not at the beginning of the frame history, then advance backward one frame.
		// The frame history might have been refilled by the block above just before this, so use a bare if-check and not an else.
		if (m_playbackFrame != m_frameHistory.begin()) {
			--m_playbackFrame;
		}

		//Advance the segment cache
		Segment* playingSegment = nullptr;
		if (m_playingSegment < m_segments.size()) {
			playingSegment = m_segments[m_playingSegment];
		}

		if (playingSegment != nullptr && playingSegment->CanCacheMoreFrames()) {
			playingSegment->CacheFrame();
		}
	}
}



// IEmulatedDisplay

ScreenBuffer* Rewinder::GetStableScreenBuffer()
{
	std::lock_guard<std::mutex> scopedLock(m_frameHistoryLock);

	if (m_isRewinding == false || m_frameHistory.empty() || m_playbackFrame == m_frameHistory.end())
	{
		//Not rewinding.  Just pass through.
		return MachineFeature::GetStableScreenBuffer();
	}
	else
	{
		//Rewinding.  Return our current history frame.
		return m_playbackFrame->Screen;
	}
}

int Rewinder::GetScreenBufferCount()
{
	std::lock_guard<std::mutex> scopedLock(m_frameHistoryLock);

	if (m_isRewinding == false || m_frameHistory.empty() || m_playbackFrame == m_frameHistory.end())
	{
		//Not rewinding.  Just pass through.
		return MachineFeature::GetScreenBufferCount();
	}
	else
	{
		//Rewinding.  Return our current history frame's id.
		return m_playbackFrame->ScreenBufferId;
	}
}


// IEmulatedSound

AudioBuffer Rewinder::GetStableAudioBuffer()
{
	std::lock_guard<std::mutex> scopedLock(m_frameHistoryLock);

	if (m_isRewinding == false || m_frameHistory.empty() || m_playbackFrame == m_frameHistory.end())
	{
		//Not rewinding.  Just pass through.
		return MachineFeature::GetStableAudioBuffer();
	}
	else
	{
		//Rewinding.  Return our current history frame.
		return m_playbackFrame->Audio;
	}
}

int Rewinder::GetAudioBufferCount()
{
	std::lock_guard<std::mutex> scopedLock(m_frameHistoryLock);

	if (m_isRewinding == false || m_frameHistory.empty() || m_playbackFrame == m_frameHistory.end())
	{
		//Not rewinding.  Just pass through.
		return MachineFeature::GetAudioBufferCount();
	}
	else
	{
		//Rewinding.  Return our current history frame's id.
		return m_playbackFrame->AudioBufferId;
	}
}

