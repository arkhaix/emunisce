#include "WaveOutSound.h"

#include "windows.h"

#include "phoenix.h"

#include "../common/machine.h"
#include "../sound/sound.h"

class WaveOutSound_Private
{
public:

	Phoenix* _Phoenix;

	Machine* _Machine;
	unsigned int _LastFramePlayed;

	bool _Mute;

	static const int _NumOutputBuffers = 3;
	static const int _NumOutputChannels = 2;	///< Mono/Stereo output

	HWAVEOUT _WaveOut;
	WAVEHDR _WaveHeader[_NumOutputBuffers];
	HANDLE _BufferFinishedEvent;

	AudioBuffer _Silence;

	AudioBuffer _AudioBuffer[_NumOutputBuffers];
	int _NextBufferIndex;

	u8 _InterleavedBuffer[_NumOutputBuffers][AudioBuffer::BufferSize * _NumOutputChannels];

	HANDLE _PlaybackThreadHandle;

	WaveOutSound_Private()
	{
		_Phoenix = NULL;
		_Machine = NULL;

		_LastFramePlayed = 0;

		_Mute = false;

		_NextBufferIndex = 0;

		for(int i=0;i<AudioBuffer::BufferSize;i++)
		{
			_Silence.Samples[0][i] = (u8)128;
			_Silence.Samples[1][i] = (u8)128;
		}
	}

	void Initialize()
	{
		InitializeWaveOut();

		_PlaybackThreadHandle = CreateThread(NULL, 0, StaticPlaybackThread, (LPVOID)this, 0, NULL);
	}

	void InitializeWaveOut()
	{
		_BufferFinishedEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		WAVEFORMATEX waveFormat;

		waveFormat.nSamplesPerSec = 22050;
		waveFormat.wBitsPerSample = 8;
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

		ShutdownWaveOut();
	}

	void ShutdownWaveOut()
	{
		waveOutClose(_WaveOut);
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


		_NextBufferIndex = 0;

		while(_Phoenix->ShutdownRequested() == false)
		{
			DWORD waitResult = WaitForSingleObject(_BufferFinishedEvent, 1000);
			if(waitResult == WAIT_ABANDONED)
				continue;

			waveOutUnprepareHeader(_WaveOut, &_WaveHeader[_NextBufferIndex], sizeof(WAVEHDR));

			if(_Mute == false && _Machine != NULL && _Machine->GetFrameCount() > _LastFramePlayed)
			{
				_AudioBuffer[_NextBufferIndex] = _Machine->GetSound()->GetStableAudioBuffer();
				_LastFramePlayed = _Machine->GetFrameCount();
			}
			else
			{
				_AudioBuffer[_NextBufferIndex] = _Silence;
			}

			InterleaveAudioBuffer(_NextBufferIndex);
			PlayAudioBuffer(_NextBufferIndex);

			IncrementBufferIndex();
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

	void IncrementBufferIndex()
	{
		_NextBufferIndex++;
		if(_NextBufferIndex >= _NumOutputBuffers)
			_NextBufferIndex = 0;
	}

	void InterleaveAudioBuffer(int index)
	{
		for(int i=0;i<AudioBuffer::BufferSize;i++)
		{
			if(_NumOutputChannels == 1)
			{
				int sample = _AudioBuffer[index].Samples[0][i];
				//sample += _AudioBuffer[index].Samples[1][i];
				//sample /= 2;

				_InterleavedBuffer[index][i] = (u8)sample;
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
		header->dwBufferLength = AudioBuffer::BufferSize * _NumOutputChannels;
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

