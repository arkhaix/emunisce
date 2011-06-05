#ifndef SOUND_H
#define SOUND_H

#include "../common/types.h"

class Sound1;
class Sound2;
class Sound3;
class Sound4;

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
	
	//Component

	Machine* m_machine;
	Memory* m_memory;

	
	//Audio

	AudioBuffer m_audioBuffer[2];
	AudioBuffer* m_activeAudioBuffer;
	AudioBuffer* m_stableAudioBuffer;
	void* m_audioBufferLock;
	int m_audioBufferCount;

	float m_ticksPerSample;
	float m_ticksUntilNextSample;
	int m_ticksSinceLastSample;

	unsigned int m_nextSampleIndex;


	//Sound generation

	bool m_hasPower;


	//Sound generators

	Sound1* m_sound1;
	Sound2* m_sound2;
	Sound3* m_sound3;
	Sound4* m_sound4;


	//Registers

	u8 m_inaccessible;	///<For registers like the unused memory before wave ram.

	u8 m_nr50;	///<ff24
	u8 m_nr51;	///<ff25
	u8 m_nr52;	///<ff26
};



#endif
