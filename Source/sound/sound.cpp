#include "sound.h"

#include "windows.h"	///<For critical sections.  The audio buffer needs to be locked.

#include "../common/machine.h"
#include "../memory/memory.h"

#if 0
#include <cstdio>
#define TRACE_REGISTER_WRITE printf(__FUNCTION__ "(%02X) nr52(%02X)\n", value, m_nr52);
#else
#define TRACE_REGISTER_WRITE
#endif

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

	m_poweringDown = false;

	m_lastSweepUpdateTimeSeconds = 0.f;
	m_lastEnvelope1UpdateTimeSeconds = 0.f;
	m_sound1Frequency = 0;

	m_lastEnvelope2UpdateTimeSeconds = 0.f;

	m_lastEnvelope4UpdateTimeSeconds = 0.f;

	m_sound1Playing = false;
	m_sound2Playing = false;
	m_sound3Playing = false;
	m_sound4Playing = false;

	m_sound1LengthUnit.SetTicksPerSecond( m_machine->GetTicksPerSecond() );
	m_sound2LengthUnit.SetTicksPerSecond( m_machine->GetTicksPerSecond() );
	m_sound3LengthUnit.SetTicksPerSecond( m_machine->GetTicksPerSecond() );
	m_sound4LengthUnit.SetTicksPerSecond( m_machine->GetTicksPerSecond() );

	m_sound1LengthUnit.SetDecrementsPerSecond(256);
	m_sound2LengthUnit.SetDecrementsPerSecond(256);
	m_sound3LengthUnit.SetDecrementsPerSecond(256);
	m_sound4LengthUnit.SetDecrementsPerSecond(256);

	m_sound1LengthUnit.SetMaxLength(64);
	m_sound2LengthUnit.SetMaxLength(64);
	m_sound3LengthUnit.SetMaxLength(256);
	m_sound4LengthUnit.SetMaxLength(64);

	m_audioBufferCount = 0;

	m_inaccessable = 0xff;

	SetNR52(0xf0);	///<Must enable the master control before setting the individual registers

	SetNR10(0x80);
	SetNR11(0x3f);
	SetNR12(0x00);
	SetNR13(0xff);
	SetNR14(0xbf);

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

	m_memory->SetRegisterLocation(0x15, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x16, &m_nr21, false);
	m_memory->SetRegisterLocation(0x17, &m_nr22, false);
	m_memory->SetRegisterLocation(0x18, &m_nr23, false);
	m_memory->SetRegisterLocation(0x19, &m_nr24, false);

	m_memory->SetRegisterLocation(0x1a, &m_nr30, false);
	m_memory->SetRegisterLocation(0x1b, &m_nr31, false);
	m_memory->SetRegisterLocation(0x1c, &m_nr32, false);
	m_memory->SetRegisterLocation(0x1d, &m_nr33, false);
	m_memory->SetRegisterLocation(0x1e, &m_nr34, false);

	m_memory->SetRegisterLocation(0x1f, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x20, &m_nr41, false);
	m_memory->SetRegisterLocation(0x21, &m_nr42, false);
	m_memory->SetRegisterLocation(0x22, &m_nr43, false);
	m_memory->SetRegisterLocation(0x23, &m_nr44, false);

	m_memory->SetRegisterLocation(0x24, &m_nr50, false);
	m_memory->SetRegisterLocation(0x25, &m_nr51, false);
	m_memory->SetRegisterLocation(0x26, &m_nr52, false);

	m_memory->SetRegisterLocation(0x27, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x28, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x29, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x2a, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x2b, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x2c, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x2d, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x2e, &m_inaccessable, false);
	m_memory->SetRegisterLocation(0x2f, &m_inaccessable, false);
}

void Sound::Run(int ticks)
{
	if(m_soundMasterEnable == true)
	{
		//Sound 1 Tick

		//Update length
		if(m_sound1LengthUnit.IsEnabled())
		{
			m_sound1LengthUnit.Run(ticks);
			if(m_sound1LengthUnit.GetCurrentLength() == 0)
			{
				m_sound1Playing = false;
				m_nr52 &= ~(0x01);
			}
			else if(m_sound1Playing == true)
				m_nr52 |= 0x01;
		}

		if(m_sound1Playing)
		{
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
		}

		//Sound 2 Tick

		//Update length
		if(m_sound2LengthUnit.IsEnabled())
		{
			m_sound2LengthUnit.Run(ticks);
			if(m_sound2LengthUnit.GetCurrentLength() == 0)
			{
				m_sound2Playing = false;
				m_nr52 &= ~(0x02);
			}
			else if(m_sound2Playing == true)
				m_nr52 |= 0x02;
		}

		if(m_sound2Playing)
		{
			//Update envelope
			if(m_envelope2Enabled && m_totalSeconds - m_lastEnvelope2UpdateTimeSeconds >= m_envelope2StepTimeSeconds)
			{
				m_lastEnvelope2UpdateTimeSeconds = m_totalSeconds;

				if(m_envelope2Increasing == true && m_envelope2Value < 0x0f)
					m_envelope2Value++;
				else if(m_envelope2Increasing == false && m_envelope2Value > 0x00)
					m_envelope2Value--;
			}
		}

		//Sound 3 Tick

		//Update length
		if(m_sound3LengthUnit.IsEnabled())
		{
			m_sound3LengthUnit.Run(ticks);
			if(m_sound3LengthUnit.GetCurrentLength() == 0)
			{
				m_sound3Playing = false;
				m_nr52 &= ~(0x04);
			}
			else if(m_sound3Playing == true)
				m_nr52 |= 0x04;
		}

		//Sound 4 Tick

		//Update length
		if(m_sound4LengthUnit.IsEnabled())
		{
			m_sound4LengthUnit.Run(ticks);
			if(m_sound4LengthUnit.GetCurrentLength() == 0)
			{
				m_sound4Playing = false;
				m_nr52 &= ~(0x08);
			}
			else if(m_sound4Playing == true)
				m_nr52 |= 0x08;
		}

		if(m_sound4Playing == true)
		{
			//Update envelope
			if(m_envelope4Enabled && m_totalSeconds - m_lastEnvelope4UpdateTimeSeconds >= m_envelope4StepTimeSeconds)
			{
				m_lastEnvelope4UpdateTimeSeconds = m_totalSeconds;

				if(m_envelope4Increasing == true && m_envelope4Value < 0x0f)
					m_envelope4Value++;
				else if(m_envelope4Increasing == false && m_envelope4Value > 0x00)
					m_envelope4Value--;
			}

			//Update shift register
			m_sound4TicksUntilNextShift -= ticks;
			while(m_sound4TicksUntilNextShift <= 0)
			{
				m_sound4TicksUntilNextShift += m_sound4TicksPerShift;

				int a = (m_sound4ShiftRegister & 1) ? 1 : 0;
				int b = (m_sound4ShiftRegister & (1<<m_sound4ShiftRegisterTap)) ? 1 : 0;
				m_sound4ShiftRegister >>= 1;

				int result = a ^ b;
				if(result)
					m_sound4ShiftRegister |= (1<<m_sound4ShiftRegisterWidth);

				m_sound4ShiftRegisterOutput = a;
			}
		}
	}	//< if(m_soundMasterEnable == true)


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


		//Sound 1 Sample
		if(m_soundMasterEnable && m_sound1Playing)
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


		//Sound 2 Sample
		if(m_soundMasterEnable && m_sound2Playing)
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


		//Sound 3 Sample
		if(m_soundMasterEnable && m_sound3Playing)
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


		//Sound 4 Sample
		if(m_sound4Playing)
		{
			float actualAmplitude = (float)m_envelope4Value / (float)0x0f;

			float sample = -1.f;
			if(m_sound4ShiftRegisterOutput)
				sample = 1.f;

			sample *= actualAmplitude;

			sampleValue[3] = sample;
		}


		//Debug
		//for(int i=0;i<2;i++)
		//{
		//	m_terminalOutputs[i][0] = false;
		//	m_terminalOutputs[i][1] = false;
		//	m_terminalOutputs[i][2] = false;
		//	m_terminalOutputs[i][3] = true;
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
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

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
	TRACE_REGISTER_WRITE

	if(m_poweringDown == false)	///<DMG only?
		m_sound1LengthUnit.SetInverseLength(value & 0x3f);

	if(m_soundMasterEnable == false)
		return;

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
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

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

	if(m_envelope1Value == 0)
	{
		m_sound1Playing = false;
		m_nr52 &= ~(0x01);
	}

	m_nr12 = value;
}

void Sound::SetNR13(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	m_sound1Frequency &= 0x700;
	m_sound1Frequency |= value;

	m_nr13 = 0xff;
}

void Sound::SetNR14(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	m_sound1Frequency &= 0x0ff;
	m_sound1Frequency |= (value & 0x07) << 8;

	if(value & 0x40)
		m_sound1LengthUnit.Enable();
	else
		m_sound1LengthUnit.Disable();

	if(value & 0x80)
	{
		m_sound1Playing = true;
		m_envelope1Value = m_envelope1InitialValue;

		if(m_sound1LengthUnit.GetCurrentLength() == 0)
			m_sound1LengthUnit.SetInverseLength(0);
	}

	m_nr14 = value & 0x40;
	m_nr14 |= 0xbf;
}

void Sound::SetNR21(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_poweringDown == false)	///<DMG only?
		m_sound2LengthUnit.SetInverseLength(value & 0x3f);

	if(m_soundMasterEnable == false)
		return;

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
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

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

	if(m_envelope2Value == 0)
	{
		m_sound2Playing = false;
		m_nr52 &= ~(0x02);
	}

	m_nr22 = value;
}

void Sound::SetNR23(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	m_sound2Frequency &= 0x700;
	m_sound2Frequency |= value;

	m_nr23 = 0xff;
}

void Sound::SetNR24(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	m_sound2Frequency &= 0x0ff;
	m_sound2Frequency |= (value & 0x07) << 8;

	if(value & 0x40)
		m_sound2LengthUnit.Enable();
	else
		m_sound2LengthUnit.Disable();

	if(value & 0x80)
	{
		m_sound2Playing = true;
		m_envelope2Value = m_envelope2InitialValue;

		if(m_sound2LengthUnit.GetCurrentLength() == 0)
			m_sound2LengthUnit.SetInverseLength(0);
	}

	m_nr24 = value & 0x40;
	m_nr24 |= 0xbf;
}

void Sound::SetNR30(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	if(value & 0x80)
	{
		m_sound3Off = false;
	}
	else
	{
		m_sound3Off = true;

		m_sound3Playing = false;
		m_nr52 &= ~(0x04);
	}

	m_nr30 = value & 0x80;
	m_nr30 |= 0x7f;
}

void Sound::SetNR31(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_poweringDown == false)	///<DMG only?
		m_sound3LengthUnit.SetInverseLength(value);

	if(m_soundMasterEnable == false)
		return;

	m_nr31 = 0xff;
}

void Sound::SetNR32(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	m_sound3Level = (value & 0x60) >> 5;

	m_nr32 = value & 0x60;
	m_nr32 |= 0x9f;
}

void Sound::SetNR33(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	m_sound3Frequency &= 0x700;
	m_sound3Frequency |= value;

	m_nr33 = 0xff;
}

void Sound::SetNR34(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	m_sound3Frequency &= 0x0ff;
	m_sound3Frequency |= (value & 0x07) << 8;

	if(value & 0x40)
		m_sound3LengthUnit.Enable();
	else
		m_sound3LengthUnit.Disable();

	if(value & 0x80)
	{
		m_sound3Playing = true;

		if(m_sound3LengthUnit.GetCurrentLength() == 0)
			m_sound3LengthUnit.SetInverseLength(0);
	}

	m_nr34 = value & 0x40;
	m_nr34 |= 0xbf;
}

void Sound::SetNR41(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_poweringDown == false)	///<DMG only?
		m_sound4LengthUnit.SetInverseLength(value & 0x3f);

	if(m_soundMasterEnable == false)
		return;

	m_nr41 = 0xff;
}

void Sound::SetNR42(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

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

	if(m_envelope4Value == 0)
	{
		m_sound4Playing = false;
		m_nr52 &= ~(0x08);
	}

	m_nr42 = value;
}

void Sound::SetNR43(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	if(value & 0x08)
	{
		m_sound4ShiftRegisterWidth = 7;
		m_sound4ShiftRegisterTap = 1;//3;	//(1 is most tone-like, 3 is most white-noise-like)
	}
	else
	{
		m_sound4ShiftRegisterWidth = 15;
		m_sound4ShiftRegisterTap = 7;
	}


	unsigned int frequencyDivisionRatio = value & 0x07;
	unsigned int shiftClockFrequency = (value & 0xf0) >> 4;

	float shiftFrequency = (float)m_machine->GetTicksPerSecond() / 4.f;
	shiftFrequency /= (float)(frequencyDivisionRatio + 1);
	shiftFrequency /= (float)(1<<(shiftClockFrequency+1));

	m_sound4TicksPerShift = (int)( ((float)m_machine->GetTicksPerSecond() / shiftFrequency) + 0.5f );
	m_sound4TicksUntilNextShift = m_sound4TicksPerShift;

	m_nr43 = value;
}

void Sound::SetNR44(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	if(value & 0x40)
		m_sound4LengthUnit.Enable();
	else
		m_sound4LengthUnit.Disable();

	if(value & 0x80)
	{
		m_sound4Playing = true;
		m_envelope4Value = m_envelope4InitialValue;
		m_sound4ShiftRegister = 0xffff;

		if(m_sound4LengthUnit.GetCurrentLength() == 0)
			m_sound4LengthUnit.SetInverseLength(0);
	}

	m_nr44 = value & 0x40;
	m_nr44 |= 0xbf;
}

void Sound::SetNR50(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

	m_nr50 = value;
}

void Sound::SetNR51(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_soundMasterEnable == false)
		return;

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
	TRACE_REGISTER_WRITE

	if(value & 0x80)
	{
		m_soundMasterEnable = true;
	}
	else
	{
		m_poweringDown = true;

		SetNR10(0);
		SetNR11(0);
		SetNR12(0);
		SetNR13(0);
		SetNR14(0);

		SetNR21(0);
		SetNR22(0);
		SetNR23(0);
		SetNR24(0);

		SetNR30(0);
		SetNR31(0);
		SetNR32(0);
		SetNR33(0);
		SetNR34(0);

		SetNR41(0);
		SetNR42(0);
		SetNR43(0);
		SetNR44(0);

		SetNR50(0);
		SetNR51(0);

		m_poweringDown = false;
		
		m_sound1Playing = false;
		m_sound2Playing = false;
		m_sound3Playing = false;
		m_sound4Playing = false;

		m_soundMasterEnable = false;
		m_nr52 = 0x70;
	}
	
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
