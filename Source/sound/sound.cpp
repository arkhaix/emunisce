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
}

void Sound::SetMachine(Machine* machine)
{
	m_machine = machine;
	m_memory = machine->GetMemory();

	m_ticksPerSample = (float)machine->GetTicksPerSecond() / 44100.f;
	m_ticksUntilNextSample = m_ticksPerSample;
	m_ticksSinceLastSample = 0;


	m_memory->SetRegisterLocation(0x10, &m_sound1Sweep, true);
	m_memory->SetRegisterLocation(0x11, &m_sound1Length, true);
	m_memory->SetRegisterLocation(0x12, &m_sound1Envelope, true);
	m_memory->SetRegisterLocation(0x13, &m_sound1FrequencyLow, true);
	m_memory->SetRegisterLocation(0x14, &m_sound1FrequencyHigh, true);

	m_memory->SetRegisterLocation(0x16, &m_sound2Length, true);
	m_memory->SetRegisterLocation(0x17, &m_sound2Envelope, true);
	m_memory->SetRegisterLocation(0x18, &m_sound2FrequencyLow, true);
	m_memory->SetRegisterLocation(0x19, &m_sound2FrequencyHigh, true);

	m_memory->SetRegisterLocation(0x1a, &m_sound3Enable, true);
	m_memory->SetRegisterLocation(0x1b, &m_sound3Length, true);
	m_memory->SetRegisterLocation(0x1c, &m_sound3Level, true);
	m_memory->SetRegisterLocation(0x1d, &m_sound3FrequencyLow, true);
	m_memory->SetRegisterLocation(0x1e, &m_sound3FrequencyHigh, true);

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
		m_fractionalSeconds = m_totalSeconds - (int)m_totalSeconds;

		u8 sampleValue[4];
		bool channelEnabled[4];
		for(int i=0;i<4;i++)
		{
			sampleValue[i] = (u8)0x80;	///<silent
			channelEnabled[i] = false;
		}

		//Update the generators

		//Sound 1 Tick
		if(m_sound1Enabled)
		{
			if(m_sound1Playing)
			{
				//Update sound time
				if(m_totalSeconds - m_sound1StartTimeSeconds >= m_sound1LengthSeconds)
				{
					m_sound1Playing = false;
					m_soundEnable &= ~(0x01);
				}

				//Update sweep
				if(m_sweepEnabled && m_totalSeconds - m_lastSweepUpdateTimeSeconds >= m_sweepStepTimeSeconds)
				{
					m_lastSweepUpdateTimeSeconds += m_totalSeconds - m_lastSweepUpdateTimeSeconds;

					if(m_sweepIncreasing == true)
						m_sound1Frequency <<= m_sweepShift;
					else
						m_sound1Frequency >>= m_sweepShift;

					if(m_sound1Frequency >= 0x800)	///<Frequency has maximum 11-bits.  Overflow turns off the sound
					{
						m_sound1Playing = false;
						m_soundEnable &= ~(0x01);
					}
				}

				//Update envelope
				if(m_totalSeconds - m_lastEnvelope1UpdateTimeSeconds >= m_envelope1StepTimeSeconds)
				{
					m_lastEnvelope1UpdateTimeSeconds += m_totalSeconds - m_lastEnvelope1UpdateTimeSeconds;

					if(m_envelope1Increasing == true && m_envelope1Value < 0x0f)
						m_envelope1Value++;
					else if(m_envelope1Value == false && m_envelope1Value > 0x00)
						m_envelope1Value--;
				}

				//Get sample
				if(m_sound1Playing)
				{
					float actualFrequency = (float)((2048 - m_sound1Frequency) << 5);
					float actualAmplitude = (float)m_envelope1Value / (float)0x0f;

					//Need to find the position on the wave at our current time

					//If the wave is plotted on a graph, X is the independent variable (time, frequency)
					// and Y is the dependent variable (value, amplitude)

					//Frequency is always a whole number in this case (integer), so at the beginning
					// of each second, a wave is beginning.  0.00s = 1.00s = 2.00s, etc.
					// So all we need is the fractional part.
					//waveX = m_totalSeconds - (int)m_totalSeconds is the same as...
					float waveX = m_fractionalSeconds;

					//The frequency is the number of full wave periods within a single second.
					// (frequency = periods / 1 second)
					//So the duration of each period of the wave is 1 / frequency, measured in seconds.
					// (periodDuration = 1 second / frequency)
					float periodTime = 1.f / actualFrequency;

					//At the beginning of each period, we're at the beginning of a wave.
					// But all we're interested in is the fractional part of the period.
					//So we need to remove out the whole periods the same as we removed the whole seconds above.
					float numWholePeriods = waveX / periodTime;
					waveX -= numWholePeriods;

					//Note: I'm pretty sure all this period stuff can be algebraically reduced to...
					//waveX = m_fractionalSeconds * actualFrequency;
					//waveX -= (int)waveX

					//So now waveX is the fractional part of the period of the wave we're in.
					//The wave cycles high (1.f * amplitude) and low (-1.f * amplitude) based on the duty cycles.
					//Each wave starts high the beginning of the period, stays high for (duty cycle * period) seconds,
					// then goes low for the rest of the period.
					//fSample here is the Y value on our graph.
					float fSample = 1.f * actualAmplitude;
					if(waveX > m_sound1DutyCycles)
						fSample = -fSample;

					//All that's left now is to convert this sample from [-1,+1] to the [0,255] range of a byte.
					// Where 128 = silence (actually 127.5, but we can't really do that with a u8)
					u8 sample = (u8)(128 + (fSample * 127.f));
					sampleValue[0] = sample;
				}
			}
		}

		//Get the samples

		//Mix the samples

		//Output the final sample

		//Test code to make sure the sound path is working.  To be removed.
		//-->
		float go = m_totalSeconds * 100.f;
		float fractionalPart = go - (int)go;
		if(fractionalPart >= .5f)
		{
			m_activeAudioBuffer->Samples[0][m_nextSampleIndex] = (u8)200;
			m_activeAudioBuffer->Samples[1][m_nextSampleIndex] = (u8)200;
		}
		else
		{
			m_activeAudioBuffer->Samples[0][m_nextSampleIndex] = (u8)55;
			m_activeAudioBuffer->Samples[1][m_nextSampleIndex] = (u8)55;
		}
		//<--
		//End test code

		//Update the index
		m_nextSampleIndex++;
		if(m_nextSampleIndex >= AudioBuffer::BufferSize)
		{
			//Reached the end of the buffer.  Swap them.
			EnterCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
				AudioBuffer* temp = m_stableAudioBuffer;
				m_stableAudioBuffer = m_activeAudioBuffer;
				m_activeAudioBuffer = temp;
			LeaveCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);

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
