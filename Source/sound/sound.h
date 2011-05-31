#ifndef SOUND_H
#define SOUND_H

#include "../common/types.h"

static const unsigned int SamplesPerSecond = 22050;

static const unsigned int BytesPerSample = 1;
typedef u8 SampleType;
#define SilentSample ((SampleType)0x80)
#define MaxSample ((SampleType)0xff)

//static const unsigned int BytesPerSample = 2;
//typedef u16 SampleType;
//#define SilentSample ((SampleType)0x80)
//#define MaxSample ((SampleType)0xff)

struct AudioBuffer
{
	static const unsigned int BufferSizeSamples = SamplesPerSecond / 10;	///<10fps
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

private:
	
	SampleType Mix(SampleType a, SampleType b);

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

	float m_totalSeconds;	///<Total time elapsed in seconds
	float m_fractionalSeconds;	///<Fractional part of m_totalSeconds


	//Registers

	u8 m_nr10;	///<NR10, ff10
	u8 m_nr11;	///<NR11, ff11
	u8 m_nr12;	///<NR12, ff12
	u8 m_nr13;	///<NR13, ff13
	u8 m_nr14;	///<NR14, ff14

	u8 m_nr21;	///<NR21, ff16
	u8 m_nr22;	///<NR22, ff17
	u8 m_nr23;	///<NR23, ff18
	u8 m_nr24;	///<NR24, ff19

	u8 m_nr30;	///<NR30, ff1a
	u8 m_nr31;	///<NR31, ff1b
	u8 m_nr32;	///<NR32, ff1c
	u8 m_nr33;	///<NR33, ff1d
	u8 m_nr34;	///<NR34, ff1e

	u8 m_sound4Length;			///<NR41, ff20
	u8 m_sound4Envelope;		///<NR42, ff21
	u8 m_sound4Nfc;				///<NR43, ff22
	u8 m_sound4Initialize;		///<NR44, ff23

	u8 m_soundOutputLevels;		///<NR50, ff24
	u8 m_soundOutputTerminals;	///<NR51, ff25
	u8 m_soundEnable;			///<NR52, ff26


	//Useful things

	bool m_sound1Enabled;
	bool m_sound1Playing;
	bool m_sound1Continuous;

	float m_sound1StartTimeSeconds;
	float m_sound1LengthSeconds;

	float m_sound1DutyCycles;	///<[0,1]

	int m_sound1Frequency;
	float m_lastSweepUpdateTimeSeconds;
	float m_sweepStepTimeSeconds;
	int m_sweepShift;
	bool m_sweepIncreasing;

	bool m_envelope1Enabled;
	float m_lastEnvelope1UpdateTimeSeconds;
	float m_envelope1StepTimeSeconds;
	bool m_envelope1Increasing;
	int m_envelope1Value;
	int m_envelope1InitialValue;



	bool m_sound2Enabled;
	bool m_sound2Playing;
	bool m_sound2Continuous;

	float m_sound2StartTimeSeconds;
	float m_sound2LengthSeconds;

	float m_sound2DutyCycles;	///<[0,1]

	int m_sound2Frequency;

	bool m_envelope2Enabled;
	float m_lastEnvelope2UpdateTimeSeconds;
	float m_envelope2StepTimeSeconds;
	bool m_envelope2Increasing;
	int m_envelope2Value;
	int m_envelope2InitialValue;


	bool m_sound3Enabled;
	bool m_sound3Playing;
	bool m_sound3Off;
	bool m_sound3Continuous;

	float m_sound3StartTimeSeconds;
	float m_sound3LengthSeconds;

	int m_sound3Frequency;
	int m_sound3Level;
};



#endif
