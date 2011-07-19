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
#include "WaveOutSound.h"
using namespace Emunisce;

#include "windows.h"

#include "BaseApplication/MachineRunner.h"
#include "../Emunisce/Emunisce.h"	///<todo: this is just here for requesting shutdown?  refactor this.

#include "MachineIncludes.h"

#include <iostream>
#include <queue>
using namespace std;


namespace Emunisce
{

class WaveOutSound_Private
{
public:

	EmunisceApplication* _Phoenix;

	IEmulatedMachine* _Machine;
	unsigned int _LastFrameQueued;

	bool _Mute;

	static const int _NumOutputBuffers = 3;
	static const int _NumOutputChannels = 2;	///< Mono/Stereo output

	HWAVEOUT _WaveOut;
	WAVEHDR _WaveHeader[_NumOutputBuffers];
	HANDLE _BufferFinishedEvent;

	AudioBuffer _Silence;

	AudioBuffer _AudioBuffer[_NumOutputBuffers];

	SampleType _InterleavedBuffer[_NumOutputBuffers][AudioBuffer::BufferSizeSamples * _NumOutputChannels];

	HANDLE _PlaybackThreadHandle;

	queue<AudioBuffer> _PendingBufferQueue;
	queue<bool> _PendingBufferIsOverflow;
	CRITICAL_SECTION _PendingBufferQueueLock;
	HANDLE _MonitorThreadHandle;

	WaveOutSound_Private()
	{
		_Phoenix = NULL;
		_Machine = NULL;

		_LastFrameQueued = 0;

		_Mute = false;

		for(unsigned int i=0;i<AudioBuffer::BufferSizeSamples;i++)
		{
			_Silence.Samples[0][i] = SilentSample;
			_Silence.Samples[1][i] = SilentSample;
		}
	}

	void Initialize()
	{
		InitializeCriticalSection(&_PendingBufferQueueLock);

		InitializeWaveOut();

		_MonitorThreadHandle = CreateThread(NULL, 0, StaticMonitorThread, (LPVOID)this, 0, NULL);
		_PlaybackThreadHandle = CreateThread(NULL, 0, StaticPlaybackThread, (LPVOID)this, 0, NULL);
	}

	void InitializeWaveOut()
	{
		_BufferFinishedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		WAVEFORMATEX waveFormat;

		waveFormat.nSamplesPerSec = SamplesPerSecond;
		waveFormat.wBitsPerSample = 8 * BytesPerSample;
		waveFormat.nChannels = _NumOutputChannels;

		waveFormat.cbSize = 0;
		waveFormat.wFormatTag = WAVE_FORMAT_PCM;
		waveFormat.nBlockAlign = (waveFormat.wBitsPerSample >> 3) * waveFormat.nChannels;
		waveFormat.nAvgBytesPerSec = waveFormat.nBlockAlign * waveFormat.nSamplesPerSec;

		waveOutOpen(&_WaveOut, WAVE_MAPPER, &waveFormat, (DWORD_PTR)_BufferFinishedEvent, 0, CALLBACK_EVENT);
	}

	void Shutdown()
	{
		_Phoenix->RequestShutdown();
		WaitForSingleObject(_PlaybackThreadHandle, 1000);
		WaitForSingleObject(_MonitorThreadHandle, 1000);

		ShutdownWaveOut();

		DeleteCriticalSection(&_PendingBufferQueueLock);
	}

	void ShutdownWaveOut()
	{
		waveOutClose(_WaveOut);
	}

	DWORD MonitorThread()
	{
		while(_Machine == NULL && _Phoenix->ShutdownRequested() == false)
			Sleep(100);

		if(_Phoenix->ShutdownRequested())
			return 0;

		while(_Phoenix->ShutdownRequested() == false)
		{
			if(_Mute == false && _Machine != NULL && _Machine->GetSound()->GetAudioBufferCount() != (int)_LastFrameQueued)
			{
				AudioBuffer buffer = _Machine->GetSound()->GetStableAudioBuffer();

				EnterCriticalSection(&_PendingBufferQueueLock);
					_PendingBufferQueue.push(buffer);
					_PendingBufferIsOverflow.push(false);

					while(_PendingBufferIsOverflow.empty() == false && _PendingBufferIsOverflow.front() == true)
					{
						_PendingBufferIsOverflow.pop();
						_PendingBufferQueue.pop();
					}

					//Only keep the 3 most recent buffers.  This keeps sound from lagging too far behind (and staying that way).
					float emulationSpeed = _Phoenix->GetMachineRunner()->GetEmulationSpeed();
					if(emulationSpeed <= (0.0 + 1e-5) || emulationSpeed >= (1.0 - 1e-5))
					{
						while(_PendingBufferQueue.size() > 3)
						{
							_PendingBufferQueue.pop();
							_PendingBufferIsOverflow.pop();
						}
					}
				LeaveCriticalSection(&_PendingBufferQueueLock);

				_LastFrameQueued = _Machine->GetSound()->GetAudioBufferCount();
			}
			else
			{
				Sleep(1);
			}
		}

		return 0;
	}

	static DWORD WINAPI StaticMonitorThread(LPVOID param)
	{
		WaveOutSound_Private* instance = (WaveOutSound_Private*)param;
		if(instance == NULL)
			return 1;

		return instance->MonitorThread();
	}

	DWORD PlaybackThread()
	{
		while(_Machine == NULL && _Phoenix->ShutdownRequested() == false)
			Sleep(100);

		if(_Phoenix->ShutdownRequested())
			return 0;


		int numPendingBuffers = 0;
		while(numPendingBuffers == 0 && _Phoenix->ShutdownRequested() == false)
		{
			EnterCriticalSection(&_PendingBufferQueueLock);
				numPendingBuffers = (int)_PendingBufferQueue.size();
			LeaveCriticalSection(&_PendingBufferQueueLock);

			if(numPendingBuffers == 0)
				Sleep(1);
		}

		if(_Phoenix->ShutdownRequested())
			return 0;


		//Initialize the playback variables

		SetEvent(_BufferFinishedEvent);	///<No playback has occurred yet, so waveOutWrite hasn't set the event on its own yet, and we don't want to get stuck on WaitForSingleObject the first time through

		for(int i=0;i<_NumOutputBuffers;i++)
			_WaveHeader[i].dwFlags |= WHDR_DONE;	///<No playback has occurred yet, so this flag hasn't been set yet for any of the headers, but they're all really available.


		//The juice is here

		while(_Phoenix->ShutdownRequested() == false)
		{
			DWORD waitResult = WaitForSingleObject(_BufferFinishedEvent, 1000);
			if(waitResult == WAIT_ABANDONED)
				continue;

			int numPendingBuffers = 0;
			while(numPendingBuffers == 0 && _Phoenix->ShutdownRequested() == false)
			{
				EnterCriticalSection(&_PendingBufferQueueLock);
					numPendingBuffers = (int)_PendingBufferQueue.size();
				LeaveCriticalSection(&_PendingBufferQueueLock);

				if(numPendingBuffers == 0)
					Sleep(1);
			}


			for(int i=0;i<numPendingBuffers;i++)
			{
				for(int j=0;j<_NumOutputBuffers;j++)
				{
					if(_WaveHeader[j].dwFlags & WHDR_DONE)
					{
						waveOutUnprepareHeader(_WaveOut, &_WaveHeader[j], sizeof(WAVEHDR));

						bool pendingBufferQueueEmpty = false;

						EnterCriticalSection(&_PendingBufferQueueLock);
							_AudioBuffer[j] = _PendingBufferQueue.front();
							_PendingBufferQueue.pop();

							bool isOverflow = _PendingBufferIsOverflow.front();
							_PendingBufferIsOverflow.pop();

							pendingBufferQueueEmpty = _PendingBufferQueue.empty();
						LeaveCriticalSection(&_PendingBufferQueueLock);

						float emulationSpeed = _Phoenix->GetMachineRunner()->GetEmulationSpeed();
						if(emulationSpeed > 0.f && emulationSpeed < 1.f && pendingBufferQueueEmpty == true && isOverflow == false)
						{
							AudioBuffer overflowBuffer;
							overflowBuffer.NumSamples = 0;

							float oldSamplesPerNewSample = emulationSpeed;
							float fOldSampleIndex = 0.f;
							unsigned int oldSampleIndex = 0;

							unsigned int newSampleIndex = 0;

							EnterCriticalSection(&_PendingBufferQueueLock);

							while(oldSampleIndex < _AudioBuffer[j].NumSamples)
							{
								overflowBuffer.Samples[0][newSampleIndex] = _AudioBuffer[j].Samples[0][oldSampleIndex];
								overflowBuffer.Samples[1][newSampleIndex] = _AudioBuffer[j].Samples[1][oldSampleIndex];

								fOldSampleIndex += oldSamplesPerNewSample;
								oldSampleIndex = (int)fOldSampleIndex;
								newSampleIndex++;

								if(newSampleIndex >= AudioBuffer::BufferSizeSamples)
								{
									overflowBuffer.NumSamples = AudioBuffer::BufferSizeSamples;
									_PendingBufferQueue.push(overflowBuffer);
									_PendingBufferIsOverflow.push(true);
									newSampleIndex = 0;
								}
							}

							if(newSampleIndex > 0)
							{
								overflowBuffer.NumSamples = newSampleIndex;
								_PendingBufferQueue.push(overflowBuffer);
								_PendingBufferIsOverflow.push(true);
							}

							LeaveCriticalSection(&_PendingBufferQueueLock);

							SetEvent(_BufferFinishedEvent);
						}
						else
						{
							InterleaveAudioBuffer(j);
							PlayAudioBuffer(j);
						}

						break;
					}
				}
			}
		}

		return 0;
	}

	static DWORD WINAPI StaticPlaybackThread(LPVOID param)
	{
		WaveOutSound_Private* instance = (WaveOutSound_Private*)param;
		if(instance != NULL)
			return instance->PlaybackThread();

		return 1;
	}

	void InterleaveAudioBuffer(int index)
	{
		for(unsigned int i=0;i<_AudioBuffer[index].NumSamples;i++)
		{
			if(_NumOutputChannels == 1)
			{
				int sample = _AudioBuffer[index].Samples[0][i];
				//sample += _AudioBuffer[index].Samples[1][i];
				//sample /= 2;

				_InterleavedBuffer[index][i] = (SampleType)sample;
			}
			else if(_NumOutputChannels == 2)
			{
				_InterleavedBuffer[index][ (i*2) + 0 ] = _AudioBuffer[index].Samples[0][i];
				_InterleavedBuffer[index][ (i*2) + 1 ] = _AudioBuffer[index].Samples[1][i];
			}
		}
	}

	void PlayAudioBuffer(int index)
	{
		WAVEHDR* header = &_WaveHeader[index];
		ZeroMemory(header, sizeof(WAVEHDR));
		header->dwBufferLength = BytesPerSample * _AudioBuffer[index].NumSamples * _NumOutputChannels;
		header->lpData = (LPSTR)&_InterleavedBuffer[index][0];

		waveOutPrepareHeader(_WaveOut, header, sizeof(WAVEHDR));
		waveOutWrite(_WaveOut, header, sizeof(WAVEHDR));
	}
};

}	//namespace Emunisce


WaveOutSound::WaveOutSound()
{
	m_private = new WaveOutSound_Private();
}

WaveOutSound::~WaveOutSound()
{
	delete m_private;
}

void WaveOutSound::Initialize(EmunisceApplication* phoenix)
{
	m_private->_Phoenix = phoenix;

	m_private->Initialize();
}

void WaveOutSound::Shutdown()
{
	m_private->Shutdown();
}

void WaveOutSound::SetMachine(IEmulatedMachine* machine)
{
	m_private->_Machine = machine;
}

void WaveOutSound::SetMute(bool mute)
{
	m_private->_Mute = mute;
}

