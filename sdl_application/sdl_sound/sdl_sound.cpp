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
#include "sdl_sound.h"

#include <SDL3/SDL.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <thread>

#include "../emunisce/emunisce.h"
#include "base_application/machine_runner.h"
#include "machine_includes.h"

using namespace emunisce;

namespace emunisce {

class SDLSound_Private {
public:
	EmunisceApplication* m_phoenix;

	EmulatedMachine* m_machine;
	unsigned int m_lastFrameQueued;

	bool m_mute;

	static const int NumOutputBuffers = 3;
	static const int NumOutputChannels = 2;  ///< Stereo output

	SDL_AudioStream* m_audioStream;

	AudioBuffer m_silence;

	AudioBuffer m_audioBuffer[NumOutputBuffers];

	SampleType m_interleavedBuffer[NumOutputBuffers][AudioBuffer::BufferSizeSamples * NumOutputChannels];

	std::thread m_monitorThread;
	std::atomic<bool> m_shutdownRequested;

	std::queue<AudioBuffer> m_pendingBufferQueue;
	std::queue<bool> m_pendingBufferIsOverflow;
	std::mutex m_pendingBufferQueueMutex;

	SDLSound_Private() {
		m_phoenix = nullptr;
		m_machine = nullptr;

		m_lastFrameQueued = 0;

		m_mute = false;
		m_audioStream = nullptr;
		m_shutdownRequested = false;

		for (unsigned int i = 0; i < AudioBuffer::BufferSizeSamples; i++) {
			m_silence.Samples[0][i] = SilentSample;
			m_silence.Samples[1][i] = SilentSample;
		}
	}

	void Initialize() {
		m_shutdownRequested = false;
		InitializeSDLAudio();

		m_monitorThread = std::thread(&SDLSound_Private::MonitorThread, this);
	}

	void InitializeSDLAudio() {
		SDL_AudioSpec spec;
		spec.freq = SamplesPerSecond;
		spec.format = SDL_AUDIO_S16;  // 16-bit signed audio
		spec.channels = NumOutputChannels;

		m_audioStream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, nullptr, nullptr);
		if (m_audioStream == nullptr) {
			SDL_Log("Failed to open audio device: %s", SDL_GetError());
			return;
		}

		// Start audio playback
		SDL_ResumeAudioStreamDevice(m_audioStream);
	}

	void Shutdown() {
		m_shutdownRequested = true;

		if (m_monitorThread.joinable()) {
			m_monitorThread.join();
		}

		ShutdownSDLAudio();
	}

	void ShutdownSDLAudio() {
		if (m_audioStream != nullptr) {
			SDL_DestroyAudioStream(m_audioStream);
			m_audioStream = nullptr;
		}
	}

	void MonitorThread() {
		while (m_machine == nullptr && !m_shutdownRequested && !m_phoenix->ShutdownRequested()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}

		if (m_shutdownRequested || m_phoenix->ShutdownRequested()) {
			return;
		}

		while (!m_shutdownRequested && !m_phoenix->ShutdownRequested()) {
			if (m_mute == false && m_machine != nullptr &&
				m_machine->GetSound()->GetAudioBufferCount() != (int)m_lastFrameQueued) {
				AudioBuffer buffer = m_machine->GetSound()->GetStableAudioBuffer();

				{
					std::lock_guard<std::mutex> lock(m_pendingBufferQueueMutex);
					m_pendingBufferQueue.push(buffer);
					m_pendingBufferIsOverflow.push(false);

					while (!m_pendingBufferIsOverflow.empty() && m_pendingBufferIsOverflow.front() == true) {
						m_pendingBufferIsOverflow.pop();
						m_pendingBufferQueue.pop();
					}

					// Only keep the 3 most recent buffers. This keeps sound from lagging too far behind.
					float emulationSpeed = m_phoenix->GetMachineRunner()->GetEmulationSpeed();
					if (emulationSpeed <= (0.0 + 1e-5) || emulationSpeed >= (1.0 - 1e-5)) {
						while (m_pendingBufferQueue.size() > 3) {
							m_pendingBufferQueue.pop();
							m_pendingBufferIsOverflow.pop();
						}
					}
				}

				m_lastFrameQueued = m_machine->GetSound()->GetAudioBufferCount();

				// Process pending buffers and submit to SDL audio stream
				ProcessPendingBuffers();
			}
			else {
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}
		}
	}

	void ProcessPendingBuffers() {
		std::lock_guard<std::mutex> lock(m_pendingBufferQueueMutex);

		while (!m_pendingBufferQueue.empty()) {
			AudioBuffer buffer = m_pendingBufferQueue.front();
			m_pendingBufferQueue.pop();

			bool isOverflow = m_pendingBufferIsOverflow.front();
			m_pendingBufferIsOverflow.pop();

			bool pendingBufferQueueEmpty = m_pendingBufferQueue.empty();

			float emulationSpeed = m_phoenix->GetMachineRunner()->GetEmulationSpeed();
			if (emulationSpeed > 0.f && emulationSpeed < 1.f && pendingBufferQueueEmpty && !isOverflow) {
				// Handle slow-motion playback by stretching audio
				AudioBuffer overflowBuffer;
				overflowBuffer.NumSamples = 0;

				float oldSamplesPerNewSample = emulationSpeed;
				float fOldSampleIndex = 0.f;
				unsigned int oldSampleIndex = 0;
				unsigned int newSampleIndex = 0;

				while (oldSampleIndex < buffer.NumSamples) {
					overflowBuffer.Samples[0][newSampleIndex] = buffer.Samples[0][oldSampleIndex];
					overflowBuffer.Samples[1][newSampleIndex] = buffer.Samples[1][oldSampleIndex];

					fOldSampleIndex += oldSamplesPerNewSample;
					oldSampleIndex = (int)fOldSampleIndex;
					newSampleIndex++;

					if (newSampleIndex >= AudioBuffer::BufferSizeSamples) {
						overflowBuffer.NumSamples = AudioBuffer::BufferSizeSamples;
						m_pendingBufferQueue.push(overflowBuffer);
						m_pendingBufferIsOverflow.push(true);
						newSampleIndex = 0;
					}
				}

				if (newSampleIndex > 0) {
					overflowBuffer.NumSamples = newSampleIndex;
					m_pendingBufferQueue.push(overflowBuffer);
					m_pendingBufferIsOverflow.push(true);
				}
			}
			else {
				// Normal playback - interleave and submit
				InterleaveAndSubmitBuffer(buffer);
			}
		}
	}

	void InterleaveAndSubmitBuffer(const AudioBuffer& buffer) {
		// Use a local buffer for interleaving
		SampleType interleavedBuffer[AudioBuffer::BufferSizeSamples * NumOutputChannels];

		for (unsigned int i = 0; i < buffer.NumSamples; i++) {
			interleavedBuffer[(i * 2) + 0] = buffer.Samples[0][i];
			interleavedBuffer[(i * 2) + 1] = buffer.Samples[1][i];
		}

		// Submit to SDL audio stream
		if (m_audioStream != nullptr) {
			int bytesToWrite = BytesPerSample * buffer.NumSamples * NumOutputChannels;
			if (!SDL_PutAudioStreamData(m_audioStream, interleavedBuffer, bytesToWrite)) {
				SDL_Log("Failed to put audio data: %s", SDL_GetError());
			}
		}
	}
};

}  // namespace emunisce

SDLSound::SDLSound() {
	m_private = new SDLSound_Private();
}

SDLSound::~SDLSound() {
	delete m_private;
}

void SDLSound::Initialize(EmunisceApplication* phoenix) {
	m_private->m_phoenix = phoenix;
	m_private->Initialize();
}

void SDLSound::Shutdown() {
	m_private->Shutdown();
}

void SDLSound::SetMachine(EmulatedMachine* machine) {
	m_private->m_machine = machine;
}

void SDLSound::SetMute(bool mute) {
	m_private->m_mute = mute;
}
