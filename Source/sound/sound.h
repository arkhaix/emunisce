#ifndef SOUND_H
#define SOUND_H

#include "../common/types.h"

struct AudioBuffer
{
	static const unsigned int BufferSize = 735;	///<44100Hz / 60fps

	u8 Samples[BufferSize];	///<44100Hz / 60fps
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

	//Registers

private:

	Machine* m_machine;
	Memory* m_memory;

	AudioBuffer m_audioBuffer[2];
	AudioBuffer* m_activeAudioBuffer;
	AudioBuffer* m_stableAudioBuffer;
	void* m_audioBufferLock;

	float m_ticksPerSample;
	float m_ticksUntilNextSample;
	int m_ticksSinceLastSample;

	unsigned int m_nextSampleIndex;


	//Registers

	u8 m_sound1Sweep;			///<NR10, ff10
	u8 m_sound1Length;			///<NR11, ff11
	u8 m_sound1Envelope;		///<NR12, ff12
	u8 m_sound1FrequencyLow;	///<NR13, ff13
	u8 m_sound1FrequencyHigh;	///<NR14, ff14

	u8 m_sound2Length;			///<NR21, ff16
	u8 m_sound2Envelope;		///<NR22, ff17
	u8 m_sound2FrequencyLow;	///<NR23, ff18
	u8 m_sound2FrequencyHigh;	///<NR24, ff19

	u8 m_sound3Enable;			///<NR30, ff1a
	u8 m_sound3Length;			///<NR31, ff1b
	u8 m_sound3Level;			///<NR32, ff1c
	u8 m_sound3FrequencyLow;	///<NR33, ff1d
	u8 m_sound3FrequencyHigh;	///<NR34, ff1e

	u8 m_sound4Length;			///<NR41, ff20
	u8 m_sound4Envelope;		///<NR42, ff21
	u8 m_sound4Nfc;				///<NR43, ff22
	u8 m_sound4Initialize;		///<NR44, ff23

	u8 m_soundOutputLevels;		///<NR50, ff24
	u8 m_soundOutputTerminals;	///<NR51, ff25
	u8 m_soundEnable;			///<NR52, ff26
};

#endif
