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

	m_lastEnvelope4UpdateTimeSeconds = 0.f;

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

	SetNR41(0xff);
	SetNR42(0xff);
	SetNR43(0x00);
	SetNR44(0xbf);

	SetNR50(0x00);
	SetNR51(0x00);
	SetNR52(0xf0);
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

	m_memory->SetRegisterLocation(0x20, &m_nr41, false);
	m_memory->SetRegisterLocation(0x21, &m_nr42, false);
	m_memory->SetRegisterLocation(0x22, &m_nr43, false);
	m_memory->SetRegisterLocation(0x23, &m_nr44, false);

	m_memory->SetRegisterLocation(0x24, &m_nr50, false);
	m_memory->SetRegisterLocation(0x25, &m_nr51, false);
	m_memory->SetRegisterLocation(0x26, &m_nr52, false);
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
		if(m_soundMasterEnable && m_sound1Playing)
		{
			//Update sound time
			if(m_sound1Continuous == false && m_totalSeconds - m_sound1StartTimeSeconds >= m_sound1LengthSeconds)
			{
				m_sound1Playing = false;
				m_nr52 &= ~(0x01);
			}

			//Update sweep
			if(m_sweepShift > 0 && m_totalSeconds - m_lastSweepUpdateTimeSeconds >= m_sweepStepTimeSeconds)
			{
				m_lastSweepUpdateTimeSeconds = m_totalSeconds;

				unsigned int newFrequency = m_sound1Frequency;

				if(m_sweepIncreasing == true)
					newFrequency += (m_sound1Frequency >> m_sweepShift);
				else
					newFrequency -= (m_sound1Frequency >> m_sweepShift);

				if(newFrequency >= 0x800)	///<Frequency has maximum 11-bits.  Overflow turns off the sound
				{
					m_sound1Playing = false;
					m_nr52 &= ~(0x01);
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

				double waveX = (float)m_fractionalSeconds * actualFrequency;
				waveX -= (int)waveX;

				float fSample = 1.f * actualAmplitude;
				if(waveX > m_sound1DutyCycles)
					fSample = -fSample;

				sampleValue[0] = fSample;
			}
		}

		//Sound 2 Tick
		if(m_soundMasterEnable && m_sound2Playing)
		{
			//Update sound time
			if(m_sound2Continuous == false && m_totalSeconds - m_sound2StartTimeSeconds >= m_sound2LengthSeconds)
			{
				m_sound2Playing = false;
				m_nr52 &= ~(0x02);
			}

			//Update envelope
			if(m_envelope2Enabled && m_totalSeconds - m_lastEnvelope2UpdateTimeSeconds >= m_envelope2StepTimeSeconds)
			{
				m_lastEnvelope2UpdateTimeSeconds = m_totalSeconds;

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

				double waveX = m_fractionalSeconds * actualFrequency;
				waveX -= (int)waveX;

				float fSample = 1.f * actualAmplitude;
				if(waveX > m_sound2DutyCycles)
					fSample = -fSample;

				sampleValue[1] = fSample;
			}
		}

		//Sound 3 Tick
		if(m_soundMasterEnable && m_sound3Playing && m_sound3Off == false)
		{
			//Update sound time
			if(m_sound3Continuous == false && m_totalSeconds - m_sound3StartTimeSeconds >= m_sound3LengthSeconds)
			{
				m_sound3Playing = false;
				m_nr52 &= ~(0x04);
			}

			//Get sample
			if(m_sound3Playing)
			{
				float actualFrequency = (float)m_machine->GetTicksPerSecond() / (float)((2048 - m_sound3Frequency) << 5);

				double waveX = m_fractionalSeconds * actualFrequency;
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


		//Sound 4 Tick
		if(m_soundMasterEnable && m_sound4Playing)
		{
			//Update sound time
			if(m_sound4Continuous == false && m_totalSeconds - m_sound4StartTimeSeconds >= m_sound4LengthSeconds)
			{
				m_sound4Playing = false;
				m_nr52 &= ~(0x08);
			}

			//Update envelope
			if(m_envelope4Enabled && m_totalSeconds - m_lastEnvelope4UpdateTimeSeconds >= m_envelope4StepTimeSeconds)
			{
				m_lastEnvelope4UpdateTimeSeconds = m_totalSeconds;

				if(m_envelope4Increasing == true && m_envelope4Value < 0x0f)
					m_envelope4Value++;
				else if(m_envelope4Increasing == false && m_envelope4Value > 0x00)
					m_envelope4Value--;
			}

			//Update frequency
			m_sound4TicksUntilNextShift -= ticks;
			while(m_sound4TicksUntilNextShift <= 0)
			{
				m_sound4TicksUntilNextShift += m_sound4TicksPerShift;

				int a = m_sound4ShiftRegister & (1<<m_sound4ShiftTap);
				m_sound4ShiftRegister <<= 1;
				int b = m_sound4ShiftRegister & (1<<m_sound4ShiftTap);

				int result = a ^ b;
				if(result)
					m_sound4ShiftRegister |= 1;

				m_sound4Sample <<= 1;
				if(a)
					m_sound4Sample |= 1;
			}

			//Get sample
			if(m_sound4Playing)
			{
				float actualAmplitude = (float)m_envelope4Value / (float)0x0f;

				//float sample = (float)m_sound4Sample / (float)0x7fffffff;
				//sample -= 1.f;
				//sample *= actualAmplitude;

				float sample = -1.f;
				if(m_sound4Sample & 0x01)
					sample = 1.f;
				sample *= actualAmplitude;

				sampleValue[3] = sample;
			}
		}


		//Debug
		//for(int i=0;i<2;i++)
		//{
		//	m_terminalOutputs[i][0] = true;
		//	m_terminalOutputs[i][1] = false;
		//	m_terminalOutputs[i][2] = false;
		//	m_terminalOutputs[i][3] = false;
		//}

		//Mix the samples

		float finalSampleValue[2] = {0.f, 0.f};
		int numSampleValues[2] = {0, 0};

		for(int outputChannel=0;outputChannel<2;outputChannel++)
		{
			//Combine the 4 component channels into a final output channel value
			for(int componentChannel=0;componentChannel<4;componentChannel++)
			{
				if(m_terminalOutputs[outputChannel][componentChannel])
				{
					finalSampleValue[outputChannel] += sampleValue[componentChannel];
					numSampleValues[outputChannel]++;
				}
			}

			//Play silence if there are no components assigned to a channel
			if(numSampleValues[outputChannel] == 0)
			{
				finalSampleValue[outputChannel] = 0.f;
				numSampleValues[outputChannel] = 1;
			}

			//Divide the final value by the number of contributing components to reduce it to [-1,+1] range
			finalSampleValue[outputChannel] /= (float)numSampleValues[outputChannel];

			//In 8-bits per sample mode, the final value needs to be in the [0,1] range instead of [-1,+1]
			if(BytesPerSample == 1)
			{
				finalSampleValue[outputChannel] /= 2.f;
				finalSampleValue[outputChannel] += 0.5f;
			}
		}


		//Output the final samples

		for(int outputChannel=0;outputChannel<2;outputChannel++)
		{
			//Expand it to the sample integer range ( [0,255] or [-32768,+32767] )
			finalSampleValue[outputChannel] *= (float)MaxSample;

			//Write it to the buffer
			m_activeAudioBuffer->Samples[outputChannel][m_nextSampleIndex] = (SampleType)finalSampleValue[outputChannel];
		}
		

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

	if(value & 0x08)
		m_sweepIncreasing = false;
	else
		m_sweepIncreasing = true;

	m_sweepStepTimeSeconds = (float)((value & 0x70) >> 4) / 128.f;

	m_nr10 = value & 0x7f;
	m_nr10 |= 0x80;
}

void Sound::SetNR11(u8 value)
{
	m_sound1LengthSeconds = (float)(64 - (value & 0x3f)) * (1.f / 256.f);

	int duty = (value & 0xc0) >> 6;
	if(duty == 0)
		m_sound1DutyCycles = 0.125f;
	else
		m_sound1DutyCycles = 0.25f * duty;

	m_nr11 = value & 0xc0;
	m_nr11 |= 0x3f;
}

void Sound::SetNR12(u8 value)
{
	m_envelope1StepTimeSeconds = (value & 0x07) * (1.f / 64.f);
	if((value & 0x07) == 0)
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

	m_nr13 = 0xff;
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

	m_nr14 = value & 0x40;
	m_nr14 |= 0xbf;
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
}

void Sound::SetNR22(u8 value)
{
	m_envelope2StepTimeSeconds = (value & 0x07) * (1.f / 64.f);
	if((value & 0x07) == 0)
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
}

void Sound::SetNR30(u8 value)
{
	if(value & 0x80)
		m_sound3Off = false;
	else
		m_sound3Off = true;

	m_nr30 = value & 0x80;
	m_nr30 |= 0x7f;
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
}

void Sound::SetNR33(u8 value)
{
	m_sound3Frequency &= 0x700;
	m_sound3Frequency |= value;

	m_nr33 = 0xff;
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
}

void Sound::SetNR41(u8 value)
{
	m_sound4LengthSeconds = (double)(64 - (value & 0x3f)) * (1.0 / 64.0);

	m_nr41 = 0xff;
}

void Sound::SetNR42(u8 value)
{
	m_envelope4StepTimeSeconds = (value & 0x07) * (1.f / 64.f);
	if((value & 0x07) == 0)
		m_envelope4Enabled = false;
	else
		m_envelope4Enabled = true;

	if(value & 0x08)
		m_envelope4Increasing = true;
	else
		m_envelope4Increasing = false;

	m_envelope4InitialValue = (value & 0xf0) >> 4;
	m_envelope4Value = m_envelope4InitialValue;

	m_nr42 = value;
}

void Sound::SetNR43(u8 value)
{
	if(value & 0x40)
		m_sound4ShiftTap = 7;
	else
		m_sound4ShiftTap = 15;


	unsigned int frequencyDivisionRatio = value & 0x07;
	unsigned int shiftClockFrequency = (value & 0xf0) >> 4;

	float shiftFrequency = (float)m_machine->GetTicksPerSecond() / 4.f;
	shiftFrequency /= (float)(frequencyDivisionRatio + 1);
	shiftFrequency /= (float)(1<<(shiftClockFrequency+1));

	m_sound4TicksPerShift = (int)( (float)m_machine->GetTicksPerSecond() / shiftFrequency );
	m_sound4TicksUntilNextShift = m_sound4TicksPerShift;

	m_nr43 = value;
}

void Sound::SetNR44(u8 value)
{
	if(value & 0x40)
		m_sound4Continuous = false;
	else
		m_sound4Continuous = true;

	if(value & 0x80)
	{
		m_sound4Playing = true;
		m_sound4StartTimeSeconds = m_totalSeconds;
		m_envelope4Value = m_envelope4InitialValue;
		m_sound4ShiftRegister = 0x00ff;
	}

	m_nr44 = value & 0x40;
	m_nr44 |= 0xbf;
}

void Sound::SetNR50(u8 value)
{
	m_nr50 = value;
}

void Sound::SetNR51(u8 value)
{
	for(int i=0;i<4;i++)
	{
		if(value & (1<<i))
			m_terminalOutputs[0][i] = true;
		else
			m_terminalOutputs[0][i] = false;

		if(value & (1<<(4+i)))
			m_terminalOutputs[1][i] = true;
		else
			m_terminalOutputs[1][i] = false;
	}

	m_nr51 = value;
}

void Sound::SetNR52(u8 value)
{
	if(value & 0x80)
		m_soundMasterEnable = true;
	else
		m_soundMasterEnable = false;
	
	m_nr52 = (value & 0x80) | 0x70 | (m_nr52 & 0x0f);
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
