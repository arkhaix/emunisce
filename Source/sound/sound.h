#ifndef SOUND_H
#define SOUND_H

#include "../common/types.h"

static const unsigned int SamplesPerSecond = 48000;

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
	static const unsigned int BufferSizeSamples = SamplesPerSecond / 20;	///<Constant is frames per second
	static const unsigned int BufferSizeBytes = BufferSizeSamples * BytesPerSample;

	SampleType Samples[2][BufferSizeSamples];	///<2 channels
};

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
	
	Machine* m_machine;
	Memory* m_memory;

	AudioBuffer m_audioBuffer[2];
	AudioBuffer* m_activeAudioBuffer;
	AudioBuffer* m_stableAudioBuffer;
	void* m_audioBufferLock;
	int m_audioBufferCount;

	float m_ticksPerSample;
	float m_ticksUntilNextSample;
	int m_ticksSinceLastSample;

	unsigned int m_nextSampleIndex;


	//Registers

	u8 m_inaccessible;	///<For registers like NR20, NR40, and the unused memory before wave ram.

	u8 m_nr10;	///<ff10
	u8 m_nr11;	///<ff11
	u8 m_nr12;	///<ff12
	u8 m_nr13;	///<ff13
	u8 m_nr14;	///<ff14

	u8 m_nr21;	///<ff16
	u8 m_nr22;	///<ff17
	u8 m_nr23;	///<ff18
	u8 m_nr24;	///<ff19

	u8 m_nr30;	///<ff1a
	u8 m_nr31;	///<ff1b
	u8 m_nr32;	///<ff1c
	u8 m_nr33;	///<ff1d
	u8 m_nr34;	///<ff1e

	u8 m_nr41;	///<ff20
	u8 m_nr42;	///<ff21
	u8 m_nr43;	///<ff22
	u8 m_nr44;	///<ff23

	u8 m_nr50;	///<ff24
	u8 m_nr51;	///<ff25
	u8 m_nr52;	///<ff26
};



#endif
