#include "WaveOutSound.h"

#include "windows.h"

#include "phoenix.h"

#include "../common/machine.h"
#include "../sound/sound.h"

#include <iostream>
#include <queue>
using namespace std;

class WaveOutSound_Private
{
public:

	Phoenix* _Phoenix;

	Machine* _Machine;
	unsigned int _LastFrameQueued;

	bool _Mute;

	static const int _NumOutputBuffers = 3;
	static const int _NumOutputChannels = 1;	///< Mono/Stereo output

	HWAVEOUT _WaveOut;
	WAVEHDR _WaveHeader[_NumOutputBuffers];
	HANDLE _BufferFinishedEvent;

	AudioBuffer _Silence;

	AudioBuffer _AudioBuffer[_NumOutputBuffers];

	SampleType _InterleavedBuffer[_NumOutputBuffers][AudioBuffer::BufferSizeSamples * _NumOutputChannels];

	HANDLE _PlaybackThreadHandle;

	queue<AudioBuffer> _PendingBufferQueue;
	CRITICAL_SECTION _PendingBufferQueueLock;
	HANDLE _MonitorThreadHandle;

	WaveOutSound_Private()
	{
		_Phoenix = NULL;
		_Machine = NULL;

		_LastFrameQueued = 0;

		_Mute = false;

		for(int i=0;i<AudioBuffer::BufferSizeSamples;i++)
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
			if(_Mute == false && _Machine != NULL && _Machine->GetSound()->GetAudioBufferCount() != _LastFrameQueued)
			{
				AudioBuffer buffer = _Machine->GetSound()->GetStableAudioBuffer();

				EnterCriticalSection(&_PendingBufferQueueLock);
					_PendingBufferQueue.push(buffer);
				LeaveCriticalSection(&_PendingBufferQueueLock);

				_LastFrameQueued = _Machine->GetSound()->GetAudioBufferCount();
			}
			else
			{
				Sleep(10);
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


		for(int i=0;i<_NumOutputBuffers;i++)
		{
			_AudioBuffer[i] = _Silence;
			InterleaveAudioBuffer(i);
		}

		for(int i=0;i<_NumOutputBuffers;i++)
			PlayAudioBuffer(i);


		while(_Phoenix->ShutdownRequested() == false)
		{
			DWORD waitResult = WaitForSingleObject(_BufferFinishedEvent, 1000);
			if(waitResult == WAIT_ABANDONED)
				continue;

			for(int i=0;i<_NumOutputBuffers;i++)
			{
				if(_WaveHeader[i].dwFlags & WHDR_DONE)
				{
					waveOutUnprepareHeader(_WaveOut, &_WaveHeader[i], sizeof(WAVEHDR));

					bool usedPendingBuffer = false;

					EnterCriticalSection(&_PendingBufferQueueLock);
						if(_PendingBufferQueue.size() > 0)
						{
							_AudioBuffer[i] = _PendingBufferQueue.front();
							_PendingBufferQueue.pop();
							usedPendingBuffer = true;
						}
					LeaveCriticalSection(&_PendingBufferQueueLock);

					if(usedPendingBuffer == false)
						_AudioBuffer[i] = _Silence;

					InterleaveAudioBuffer(i);
					PlayAudioBuffer(i);
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
		for(int i=0;i<AudioBuffer::BufferSizeSamples;i++)
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
		header->dwBufferLength = AudioBuffer::BufferSizeBytes * _NumOutputChannels;
		header->lpData = (LPSTR)&_InterleavedBuffer[index][0];

		waveOutPrepareHeader(_WaveOut, header, sizeof(WAVEHDR));
		waveOutWrite(_WaveOut, header, sizeof(WAVEHDR));
	}
};

WaveOutSound::WaveOutSound()
{
	m_private = new WaveOutSound_Private();
}

WaveOutSound::~WaveOutSound()
{
	delete m_private;
}

void WaveOutSound::Initialize(Phoenix* phoenix)
{
	m_private->_Phoenix = phoenix;

	m_private->Initialize();
}

void WaveOutSound::Shutdown()
{
	m_private->Shutdown();
}

void WaveOutSound::SetMachine(Machine* machine)
{
	m_private->_Machine = machine;
}

void WaveOutSound::SetMute(bool mute)
{
	m_private->_Mute = mute;
}

