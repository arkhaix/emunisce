/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef SOUND_H
#define SOUND_H

#include "../Machine/Types.h"

class SoundGenerator;
class Sound1;
class Sound2;
class Sound3;
class Sound4;

class ChannelController;

static const unsigned int SamplesPerSecond = 44100;

//static const unsigned int BytesPerSample = 1;
//typedef u8 SampleType;
//#define SilentSample ((SampleType)0x80)
//#define MaxSample ((SampleType)0xff)

static const unsigned int BytesPerSample = 2;
typedef s16 SampleType;
#define SilentSample ((SampleType)0x0000)
#define MaxSample ((SampleType)0x7fff)

struct AudioBuffer
{
	static const unsigned int BufferSizeSamples = SamplesPerSecond / 20;	///<Constant is frames (full buffers) per second
	static const unsigned int BufferSizeBytes = BufferSizeSamples * BytesPerSample;

	SampleType Samples[2][BufferSizeSamples];	///<2 channels
};

namespace SquareSynthesisMethod
{
	typedef int Type;

	enum
	{
		Naive = 0,
		LinearInterpolation,

		NumSquareSynthesisMethods
	};
}

class Sound
{
public:

	Sound();
	~Sound();

	//Component
	void SetMachine(Machine* machine);
	void Initialize();

	void Run(int ticks);

	//External
	AudioBuffer GetStableAudioBuffer();
	int GetAudioBufferCount();

	void SetSquareSynthesisMethod(SquareSynthesisMethod::Type method);

	//Internal (for the sound generators)
	int GetFrameSequencerPosition();

	//Registers

	void SetNR10(u8 value);
	void SetNR11(u8 value);
	void SetNR12(u8 value);
	void SetNR13(u8 value);
	void SetNR14(u8 value);

	void SetNR21(u8 value);
	void SetNR22(u8 value);
	void SetNR23(u8 value);
	void SetNR24(u8 value);

	void SetNR30(u8 value);
	void SetNR31(u8 value);
	void SetNR32(u8 value);
	void SetNR33(u8 value);
	void SetNR34(u8 value);

	void SetNR41(u8 value);
	void SetNR42(u8 value);
	void SetNR43(u8 value);
	void SetNR44(u8 value);

	void SetNR50(u8 value);
	void SetNR51(u8 value);
	void SetNR52(u8 value);

private:
	

	void MixSamples(float inSamples[4], float (&outSamples)[2]);


	//Component

	Machine* m_machine;
	Memory* m_memory;

	
	//Audio

	AudioBuffer m_audioBuffer[2];
	AudioBuffer* m_activeAudioBuffer;
	AudioBuffer* m_stableAudioBuffer;
	int m_audioBufferCount;

	float m_ticksPerSample;
	float m_ticksUntilNextSample;

	unsigned int m_nextSampleIndex;


	//Frame sequencer

	int m_frameSequencerTimer;	///<Ticks remaining until the timer clocks.
	int m_frameSequencerPeriod;
	int m_frameSequencerPosition;	///<Controls the tick rates for the components (length/envelope/sweep)


	//Sound master

	bool m_hasPower;
	bool m_terminalOutputs[2][4];	///<2 output channels (stereo left/right), 4 component channels (Sound1,2,3,4)


	//Sound generators

	Sound1* m_sound1;
	Sound2* m_sound2;
	Sound3* m_sound3;
	Sound4* m_sound4;

	SoundGenerator* m_soundGenerator[4];		///<Convenience alias for iterating over m_sound1-4
	ChannelController* m_channelController[4];


	//Registers

	u8 m_inaccessible;	///<For registers like the unused memory before wave ram.

	u8 m_nr50;	///<ff24
	u8 m_nr51;	///<ff25
	u8 m_nr52;	///<ff26
};



#endif
