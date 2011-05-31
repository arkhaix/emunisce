#include "sound.h"

#include "windows.h"	///<For critical sections.  The audio buffer needs to be locked.

#include "../common/machine.h"
#include "../memory/memory.h"

Sound::Sound()
{
	m_activeAudioBuffer = &m_audioBuffer[0];
	m_stableAudioBuffer = &m_audioBuffer[1];

	m_audioBufferLock = (void*)new CRITICAL_SECTION();
	InitializeCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
}

Sound::~Sound()
{
	DeleteCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
	delete (LPCRITICAL_SECTION)m_audioBufferLock;
}

void Sound::Initialize()
{
	m_nextSampleIndex = 0;

	m_totalSeconds = 0.f;
	m_fractionalSeconds = 0.f;

	m_lastSweepUpdateTimeSeconds = 0.f;
	m_lastEnvelope1UpdateTimeSeconds = 0.f;
	m_sound1Frequency = 0;

	m_lastEnvelope2UpdateTimeSeconds = 0.f;

	m_audioBufferCount = 0;

	SetNR10(0x80);
	SetNR11(0x3f);
	SetNR12(0x00);
	SetNR13(0xff);
	SetNR14(0xbf);

	//SetNR20(0xff);
	SetNR21(0x3f);
	SetNR22(0x00);
	SetNR23(0xff);
	SetNR24(0xbf);

	SetNR30(0x7f);
	SetNR31(0xff);
	SetNR32(0x9f);
	SetNR33(0xff);
	SetNR34(0xbf);

	m_sound4Length = 0xff;		///<NR40
	m_sound4Envelope = 0xff;	///<NR41
	m_sound4Nfc = 0x00;			///<NR42
	m_sound4Initialize = 0xbf;	///<NR43

	m_soundOutputLevels = 0x00;	///<NR50
	m_soundOutputTerminals = 0x00;	///<NR51
	m_soundEnable = 0xf0;		///<NR52
}

void Sound::SetMachine(Machine* machine)
{
	m_machine = machine;
	m_memory = machine->GetMemory();

	m_ticksPerSample = (float)machine->GetTicksPerSecond() / (float)SamplesPerSecond;
	m_ticksUntilNextSample = m_ticksPerSample;
	m_ticksSinceLastSample = 0;


	m_memory->SetRegisterLocation(0x10, &m_nr10, false);
	m_memory->SetRegisterLocation(0x11, &m_nr11, false);
	m_memory->SetRegisterLocation(0x12, &m_nr12, false);
	m_memory->SetRegisterLocation(0x13, &m_nr13, false);
	m_memory->SetRegisterLocation(0x14, &m_nr14, false);

	m_memory->SetRegisterLocation(0x16, &m_nr21, false);
	m_memory->SetRegisterLocation(0x17, &m_nr22, false);
	m_memory->SetRegisterLocation(0x18, &m_nr23, false);
	m_memory->SetRegisterLocation(0x19, &m_nr24, false);

	m_memory->SetRegisterLocation(0x1a, &m_nr30, false);
	m_memory->SetRegisterLocation(0x1b, &m_nr31, false);
	m_memory->SetRegisterLocation(0x1c, &m_nr32, false);
	m_memory->SetRegisterLocation(0x1d, &m_nr33, false);
	m_memory->SetRegisterLocation(0x1e, &m_nr34, false);

	m_memory->SetRegisterLocation(0x20, &m_sound4Length, true);
	m_memory->SetRegisterLocation(0x21, &m_sound4Envelope, true);
	m_memory->SetRegisterLocation(0x22, &m_sound4Nfc, true);
	m_memory->SetRegisterLocation(0x23, &m_sound4Initialize, true);

	m_memory->SetRegisterLocation(0x24, &m_soundOutputLevels, true);
	m_memory->SetRegisterLocation(0x25, &m_soundOutputTerminals, true);
	m_memory->SetRegisterLocation(0x26, &m_soundEnable, true);
}

void Sound::Run(int ticks)
{
	m_ticksSinceLastSample += ticks;
	m_ticksUntilNextSample -= ticks;

	if(m_ticksUntilNextSample <= 0)
	{
		m_ticksUntilNextSample += m_ticksPerSample;
		
		float secondsSinceLastSample = (float)m_ticksSinceLastSample / (float)m_machine->GetTicksPerSecond();
		m_ticksSinceLastSample = 0;

		m_totalSeconds += secondsSinceLastSample;
		m_fractionalSeconds += secondsSinceLastSample;
		while(m_fractionalSeconds >= 1.f)
			m_fractionalSeconds -= 1.f;

		float sampleValue[4];
		for(int i=0;i<4;i++)
			sampleValue[i] = 0.f;


		//Update the generators

		//Sound 1 Tick
		//if(m_sound1Enabled)
		{
			if(m_sound1Playing)
			{
				//Update sound time
				if(m_sound1Continuous == false && m_totalSeconds - m_sound1StartTimeSeconds >= m_sound1LengthSeconds)
				{
					m_sound1Playing = false;
					m_soundEnable &= ~(0x01);
				}

				//Update sweep
				if(m_sweepShift > 0 && m_totalSeconds - m_lastSweepUpdateTimeSeconds >= m_sweepStepTimeSeconds)
				{
					m_lastSweepUpdateTimeSeconds = m_totalSeconds;

					int newFrequency = m_sound1Frequency;

					if(m_sweepIncreasing == true)
						newFrequency += (m_sound1Frequency >> m_sweepShift);
					else
						newFrequency -= (m_sound1Frequency >> m_sweepShift);

					if(newFrequency >= 0x800)	///<Frequency has maximum 11-bits.  Overflow turns off the sound
					{
						m_sound1Playing = false;
						m_soundEnable &= ~(0x01);
					}
					else
					{
						m_sound1Frequency = newFrequency;
					}
				}

				//Update envelope
				if(m_envelope1Enabled && m_totalSeconds - m_lastEnvelope1UpdateTimeSeconds >= m_envelope1StepTimeSeconds)
				{
					m_lastEnvelope1UpdateTimeSeconds = m_totalSeconds;

					if(m_envelope1Increasing == true && m_envelope1Value < 0x0f)
						m_envelope1Value++;
					else if(m_envelope1Increasing == false && m_envelope1Value > 0x00)
						m_envelope1Value--;
				}

				//Get sample
				if(m_sound1Playing)
				{
					float actualFrequency = (float)m_machine->GetTicksPerSecond() / (float)((2048 - m_sound1Frequency) << 5);
					float actualAmplitude = (float)m_envelope1Value / (float)0x0f;

					float waveX = m_fractionalSeconds * actualFrequency;
					waveX -= (int)waveX;

					float fSample = 1.f * actualAmplitude;
					if(waveX > m_sound1DutyCycles)
						fSample = -fSample;

					sampleValue[0] = fSample;
				}
			}
		}

		//Sound 2 Tick
		//if(m_sound2Enabled)
		{
			if(m_sound2Playing)
			{
				//Update sound time
				if(m_sound2Continuous == false && m_totalSeconds - m_sound2StartTimeSeconds >= m_sound2LengthSeconds)
				{
					m_sound2Playing = false;
					m_soundEnable &= ~(0x02);
				}

				//Update envelope
				if(m_envelope2Enabled && m_totalSeconds - m_lastEnvelope2UpdateTimeSeconds >= m_envelope2StepTimeSeconds)
				{
					m_lastEnvelope2UpdateTimeSeconds += m_totalSeconds - m_lastEnvelope2UpdateTimeSeconds;

					if(m_envelope2Increasing == true && m_envelope2Value < 0x0f)
						m_envelope2Value++;
					else if(m_envelope2Increasing == false && m_envelope2Value > 0x00)
						m_envelope2Value--;
				}

				//Get sample
				if(m_sound2Playing)
				{
					float actualFrequency = (float)m_machine->GetTicksPerSecond() / (float)((2048 - m_sound2Frequency) << 5);
					float actualAmplitude = (float)m_envelope2Value / (float)0x0f;

					float waveX = m_fractionalSeconds * actualFrequency;
					waveX -= (int)waveX;

					float fSample = 1.f * actualAmplitude;
					if(waveX > m_sound2DutyCycles)
						fSample = -fSample;

					sampleValue[1] = fSample;
				}
			}
		}

		//Sound 3 Tick
		//if(m_sound3Enabled)
		if(m_sound3Off == false)
		{
			if(m_sound3Playing)
			{
				//Update sound time
				if(m_sound3Continuous == false && m_totalSeconds - m_sound3StartTimeSeconds >= m_sound3LengthSeconds)
				{
					m_sound3Playing = false;
					m_soundEnable &= ~(0x04);
				}

				//Get sample
				if(m_sound3Playing)
				{
					float actualFrequency = (float)m_machine->GetTicksPerSecond() / (float)((2048 - m_sound3Frequency) << 5);

					float waveX = m_fractionalSeconds * actualFrequency;
					waveX -= (int)waveX;

					int sampleIndex = (int)(waveX * 32.f);
					u16 sampleAddress = 0xff30 + (sampleIndex / 2);
					SampleType sample = m_memory->Read8(sampleAddress);
					if(sampleIndex & 1)
						sample &= 0x0f;
					else
						sample = (sample & 0xf0) >> 4;

					if(m_sound3Level == 0)
						sample = 0;
					else
						sample = sample >> (m_sound3Level - 1);

					//Adjust to [0,1]
					sampleValue[2] = (float)sample / 15.f;

					//Expand to [-1,1]
					sampleValue[2] *= 2.f;
					sampleValue[2] -= 1.f;
				}
			}
		}


		//Add adjusted samples with range [0,1] instead of [-1,1]
		float adjustedSamples[4];
		for(int i=0;i<4;i++)
		{
			adjustedSamples[i] = sampleValue[i] / 2.f;
			adjustedSamples[i] += 0.5f;
		}


		//Mix the samples
		float finalSampleValue = 0.f;

		if(BytesPerSample == 1)
		{
			//for(int i=0;i<4;i++)
			//	finalSampleValue += (adjustedSamples[i]-0.5f) * mixPercentage[i];
			//finalSampleValue += 0.5f;

			for(int i=0;i<4;i++)
				finalSampleValue += adjustedSamples[i]-0.5f;
			finalSampleValue /= 4.f;
			finalSampleValue += 0.5f;

			//finalSampleValue = Mix(adjustedSamples[0], Mix(adjustedSamples[1], adjustedSamples[3]));
			//finalSampleValue = Mix(adjustedSamples[0], Mix(adjustedSamples[1], Mix(adjustedSamples[2], adjustedSamples[3])));
		}
		else if(BytesPerSample == 2)
		{
			for(int i=0;i<4;i++)
				finalSampleValue += sampleValue[i];
			finalSampleValue /= 4.f;
		}


		//Output the final sample

		if(BytesPerSample == 1 && finalSampleValue < 0.f)
			finalSampleValue = 0.f;
		else if(BytesPerSample == 2 && finalSampleValue < -1.f)
			finalSampleValue = -1.f;
		else if(finalSampleValue > 1.f)
			finalSampleValue = 1.f;

		finalSampleValue *= (float)MaxSample;

		m_activeAudioBuffer->Samples[0][m_nextSampleIndex] = (SampleType)finalSampleValue;
		m_activeAudioBuffer->Samples[1][m_nextSampleIndex] = (SampleType)finalSampleValue;
		

		//Update the index
		m_nextSampleIndex++;
		if(m_nextSampleIndex >= AudioBuffer::BufferSizeSamples)
		{
			//Reached the end of the buffer.  Swap them.
			EnterCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
				AudioBuffer* temp = m_stableAudioBuffer;
				m_stableAudioBuffer = m_activeAudioBuffer;
				m_activeAudioBuffer = temp;
			LeaveCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);

			m_audioBufferCount++;
			m_nextSampleIndex = 0;
		}
	}
}

AudioBuffer Sound::GetStableAudioBuffer()
{
	EnterCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
		AudioBuffer result = *m_stableAudioBuffer;
	LeaveCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);

	return result;
}

int Sound::GetAudioBufferCount()
{
	return m_audioBufferCount;
}

void Sound::SetNR10(u8 value)
{
	m_sweepShift = value & 0x07;
	m_envelope1Value = m_envelope1InitialValue;

	if(value & 0x08)
		m_sweepIncreasing = false;
	else
		m_sweepIncreasing = true;

	m_sweepStepTimeSeconds = (float)((value & 0x70) >> 4) / 128.f;

	//m_nr10 = value & 0x7f;
	//m_nr10 |= 0x80;
	m_nr10 = value;
}

void Sound::SetNR11(u8 value)
{
	m_sound1LengthSeconds = (float)(64 - (value & 0x3f)) * (1.f / 256.f);

	int duty = (value & 0xc0) >> 6;
	if(duty == 0)
		m_sound1DutyCycles = 0.125f;
	else
		m_sound1DutyCycles = 0.25f * duty;

	//m_nr11 = value & 0xc0;
	//m_nr11 |= 0x3f;
	m_nr11 = value;
}

void Sound::SetNR12(u8 value)
{
	m_envelope1StepTimeSeconds = (value & 0x03) * (1.f / 64.f);
	if((value & 0x03) == 0)
		m_envelope1Enabled = false;
	else
		m_envelope1Enabled = true;

	if(value & 0x08)
		m_envelope1Increasing = true;
	else
		m_envelope1Increasing = false;

	m_envelope1InitialValue = (value & 0xf0) >> 4;
	m_envelope1Value = m_envelope1InitialValue;

	m_nr12 = value;
}

void Sound::SetNR13(u8 value)
{
	m_sound1Frequency &= 0x700;
	m_sound1Frequency |= value;

	//m_nr13 = 0xff;
	m_nr13 = value;
}

void Sound::SetNR14(u8 value)
{
	m_sound1Frequency &= 0x0ff;
	m_sound1Frequency |= (value & 0x07) << 8;

	if(value & 0x40)
		m_sound1Continuous = false;
	else
		m_sound1Continuous = true;

	if(value & 0x80)
	{
		m_sound1Playing = true;
		m_sound1StartTimeSeconds = m_totalSeconds;
		m_envelope1Value = m_envelope1InitialValue;
	}

	//m_nr14 = value & 0x40;
	//m_nr14 |= 0xbf;
	m_nr14 = value;
}

void Sound::SetNR21(u8 value)
{
	m_sound2LengthSeconds = (float)(64 - (value & 0x3f)) * (1.f / 256.f);

	int duty = (value & 0xc0) >> 6;
	if(duty == 0)
		m_sound2DutyCycles = 0.125f;
	else
		m_sound2DutyCycles = 0.25f * duty;

	m_nr21 = value & 0xc0;
	m_nr21 |= 0x3f;
	m_nr21 = value;
}

void Sound::SetNR22(u8 value)
{
	m_envelope2StepTimeSeconds = (value & 0x03) * (1.f / 64.f);
	if((value & 0x03) == 0)
		m_envelope2Enabled = false;
	else
		m_envelope2Enabled = true;

	if(value & 0x08)
		m_envelope2Increasing = true;
	else
		m_envelope2Increasing = false;

	m_envelope2InitialValue = (value & 0xf0) >> 4;
	m_envelope2Value = m_envelope2InitialValue;

	m_nr22 = value;
}

void Sound::SetNR23(u8 value)
{
	m_sound2Frequency &= 0x700;
	m_sound2Frequency |= value;

	m_nr23 = 0xff;
	m_nr23 = value;
}

void Sound::SetNR24(u8 value)
{
	m_sound2Frequency &= 0x0ff;
	m_sound2Frequency |= (value & 0x07) << 8;

	if(value & 0x40)
		m_sound2Continuous = false;
	else
		m_sound2Continuous = true;

	if(value & 0x80)
	{
		m_sound2Playing = true;
		m_sound2StartTimeSeconds = m_totalSeconds;
		m_envelope2Value = m_envelope2InitialValue;
	}

	m_nr24 = value & 0x40;
	m_nr24 |= 0xbf;
	m_nr24 = value;
}

void Sound::SetNR30(u8 value)
{
	if(value & 0x80)
		m_sound3Off = false;
	else
		m_sound3Off = true;

	m_nr30 = value & 0x80;
	m_nr30 |= 0x7f;
	m_nr30 = value;
}

void Sound::SetNR31(u8 value)
{
	m_sound3LengthSeconds = (float)(256 - value) * (1.f / 256.f);

	m_nr31 = value;
}

void Sound::SetNR32(u8 value)
{
	m_sound3Level = (value & 0x60) >> 5;

	m_nr32 = value & 0x60;
	m_nr32 |= 0x9f;
	m_nr32 = value;
}

void Sound::SetNR33(u8 value)
{
	m_sound3Frequency &= 0x700;
	m_sound3Frequency |= value;

	m_nr33 = 0xff;
	m_nr33 = value;
}

void Sound::SetNR34(u8 value)
{
	m_sound3Frequency &= 0x0ff;
	m_sound3Frequency |= (value & 0x07) << 8;

	if(value & 0x40)
		m_sound3Continuous = false;
	else
		m_sound3Continuous = true;

	if(value & 0x80)
	{
		m_sound3Playing = true;
		m_sound3StartTimeSeconds = m_totalSeconds;
	}

	m_nr34 = value & 0x40;
	m_nr34 |= 0xbf;
	m_nr34 = value;
}

float Sound::Mix(float a, float b)
{
	float fResult;

	if(a < 0.5f && b < 0.5f)
		fResult = 2.f * a * b;
	else
		fResult = (2.f * (a + b)) - (2.f * a * b) - 1.f;

	return fResult;
}
