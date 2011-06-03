#ifndef SOUND_H
#define SOUND_H

#include "../common/types.h"

#include "lengthUnit.h"

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
	
	float Mix(float a, float b);

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

	double m_totalSeconds;	///<Total time elapsed in seconds
	double m_fractionalSeconds;	///<Fractional part of m_totalSeconds


	//Registers

	u8 m_inaccessable;	///<For registers like NR20, NR40, and the unused memory before wave ram.

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

	u8 m_nr41;	///<NR41, ff20
	u8 m_nr42;	///<NR42, ff21
	u8 m_nr43;	///<NR43, ff22
	u8 m_nr44;	///<NR44, ff23

	u8 m_nr50;	///<NR50, ff24
	u8 m_nr51;	///<NR51, ff25
	u8 m_nr52;	///<NR52, ff26


	//Useful things

	bool m_soundMasterEnable;
	bool m_terminalOutputs[2][4];	///<2 output channels (stereo left/right), 4 component channels (Sound1,2,3,4)


	bool m_sound1Playing;
	bool m_sound1Continuous;

	LengthUnit m_sound1LengthUnit;

	float m_sound1DutyCycles;	///<[0,1]

	int m_sound1Frequency;
	double m_lastSweepUpdateTimeSeconds;
	double m_sweepStepTimeSeconds;
	int m_sweepShift;
	bool m_sweepIncreasing;

	bool m_envelope1Enabled;
	double m_lastEnvelope1UpdateTimeSeconds;
	double m_envelope1StepTimeSeconds;
	bool m_envelope1Increasing;
	int m_envelope1Value;
	int m_envelope1InitialValue;



	bool m_sound2Playing;
	bool m_sound2Continuous;

	LengthUnit m_sound2LengthUnit;

	float m_sound2DutyCycles;	///<[0,1]

	int m_sound2Frequency;

	bool m_envelope2Enabled;
	double m_lastEnvelope2UpdateTimeSeconds;
	double m_envelope2StepTimeSeconds;
	bool m_envelope2Increasing;
	int m_envelope2Value;
	int m_envelope2InitialValue;


	bool m_sound3Playing;
	bool m_sound3Off;
	bool m_sound3Continuous;

	LengthUnit m_sound3LengthUnit;

	int m_sound3Frequency;
	int m_sound3Level;


	bool m_sound4Playing;
	bool m_sound4Continuous;

	LengthUnit m_sound4LengthUnit;

	u8 m_sound4Sample;

	bool m_envelope4Enabled;
	double m_lastEnvelope4UpdateTimeSeconds;
	double m_envelope4StepTimeSeconds;
	bool m_envelope4Increasing;
	int m_envelope4Value;
	int m_envelope4InitialValue;

	unsigned int m_sound4ShiftRegister;	///<linear feedback shift register
	int m_sound4ShiftRegisterTap;	///<tap bit for the lfsr
	int m_sound4ShiftRegisterWidth;	///<7 or 15
	int m_sound4ShiftRegisterOutput;
	int m_sound4TicksPerShift;
	int m_sound4TicksUntilNextShift;
};



#endif
