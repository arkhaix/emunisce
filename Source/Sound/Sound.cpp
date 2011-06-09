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
#include "Sound.h"

#include "windows.h"	///<For critical sections.  The audio buffer needs to be locked.

#include "../Common/Machine.h"
#include "../Memory/Memory.h"

#include "Sound1.h"
#include "Sound2.h"
#include "Sound3.h"
#include "Sound4.h"

#include "ChannelController.h"

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

	m_hasPower = true;

	for(int outputChannel=0;outputChannel<2;outputChannel++)
		for(int generatorChannel=0;generatorChannel<4;generatorChannel++)
			m_terminalOutputs[outputChannel][generatorChannel] = false;

	for(int i=0;i<4;i++)
		m_channelController[i] = new ChannelController(m_nr52, i);

	m_sound1 = new Sound1();
	m_sound2 = new Sound2();
	m_sound3 = new Sound3();
	m_sound4 = new Sound4();

	m_soundGenerator[0] = m_sound1;
	m_soundGenerator[1] = m_sound2;
	m_soundGenerator[2] = m_sound3;
	m_soundGenerator[3] = m_sound4;
}

Sound::~Sound()
{
	delete m_sound1;
	delete m_sound2;
	delete m_sound3;
	delete m_sound4;

	for(int i=0;i<4;i++)
		delete m_channelController[i];

	DeleteCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
	delete (LPCRITICAL_SECTION)m_audioBufferLock;
}

void Sound::Initialize()
{
	m_nextSampleIndex = 0;

	m_audioBufferCount = 0;

	m_frameSequencerPeriod = 8192;	///<8192 = 512Hz = (4194304 ticks per second / 512Hz).
	m_frameSequencerTimer = 0;//m_frameSequencerPeriod;
	m_frameSequencerPosition = 7;	///<auto-increments, and we want to start at 0

	m_inaccessible = 0xff;

	SetNR50(0x00);
	SetNR51(0x00);
	SetNR52(0xf0);

	m_sound1->Initialize(m_channelController[0]);
	m_sound2->Initialize(m_channelController[1]);
	m_sound3->Initialize(m_channelController[2]);
	m_sound4->Initialize(m_channelController[3]);
}

void Sound::SetMachine(Machine* machine)
{
	m_machine = machine;
	m_memory = machine->GetMemory();

	m_ticksPerSample = (float)machine->GetTicksPerSecond() / (float)SamplesPerSecond;
	m_ticksUntilNextSample = m_ticksPerSample;

	m_memory->SetRegisterLocation(0x24, &m_nr50, false);
	m_memory->SetRegisterLocation(0x25, &m_nr51, false);
	m_memory->SetRegisterLocation(0x26, &m_nr52, false);

	m_memory->SetRegisterLocation(0x27, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x28, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x29, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2a, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2b, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2c, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2d, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2e, &m_inaccessible, false);
	m_memory->SetRegisterLocation(0x2f, &m_inaccessible, false);

	m_sound1->SetMachine(machine);
	m_sound2->SetMachine(machine);
	m_sound3->SetMachine(machine);
	m_sound4->SetMachine(machine);
}

void Sound::Run(int ticks)
{
	//Update the frame sequencer

	m_frameSequencerTimer -= ticks;
	while(m_frameSequencerTimer <= 0)
	{
		m_frameSequencerTimer += m_frameSequencerPeriod;

		if(m_hasPower)
		{

			m_frameSequencerPosition++;
			if(m_frameSequencerPosition > 7)
				m_frameSequencerPosition = 0;

			if(m_frameSequencerPosition == 0 || m_frameSequencerPosition == 2 || 
				m_frameSequencerPosition == 4 || m_frameSequencerPosition == 6)
			{
				m_sound1->TickLength();
				m_sound2->TickLength();
				m_sound3->TickLength();
				m_sound4->TickLength();
			}

			if(m_frameSequencerPosition == 2 || m_frameSequencerPosition == 6)
			{
				m_sound1->TickSweep();
			}

			if(m_frameSequencerPosition == 7)
			{
				m_sound1->TickEnvelope();
				m_sound2->TickEnvelope();
				m_sound4->TickEnvelope();
			}

		}	//if(m_hasPower)
	}


	//Run the components

	m_sound1->Run(ticks);
	m_sound2->Run(ticks);
	m_sound3->Run(ticks);
	m_sound4->Run(ticks);


	//Get a sample

	m_ticksUntilNextSample -= ticks;
	while(m_ticksUntilNextSample < 0.f+1e-5)
	{
		m_ticksUntilNextSample += m_ticksPerSample;


		//Read the samples from the generators

		float sourceSamples[4] = {0.f, 0.f, 0.f, 0.f};
		
		for(int i=0;i<4;i++)
		{
			if(m_channelController[i]->IsChannelEnabled() == false)
				continue;

			sourceSamples[i] = m_soundGenerator[i]->GetSample();
		}


		//Mix the samples together

		float outputSamples[2] = {0.f, 0.f};	///<left and right channels

		MixSamples(sourceSamples, outputSamples);


		//Output the final samples

		for(int outputChannel=0;outputChannel<2;outputChannel++)
		{
			//Expand it to the sample integer range ( [0,255] or [-32768,+32767] )
			outputSamples[outputChannel] *= (float)MaxSample;

			//Write it to the buffer
			m_activeAudioBuffer->Samples[outputChannel][m_nextSampleIndex] = (SampleType)outputSamples[outputChannel];
		}


		//Update the sample index

		m_nextSampleIndex++;
		if(m_nextSampleIndex >= AudioBuffer::BufferSizeSamples)
		{
			m_nextSampleIndex = 0;

			//Swap buffers
			EnterCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);
				AudioBuffer* temp = m_stableAudioBuffer;
				m_stableAudioBuffer = m_activeAudioBuffer;
				m_activeAudioBuffer = temp;
			LeaveCriticalSection((LPCRITICAL_SECTION)m_audioBufferLock);

			m_audioBufferCount++;
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


int Sound::GetFrameSequencerPosition()
{
	return m_frameSequencerPosition;
}


void Sound::SetNR10(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR10(value);
}

void Sound::SetNR11(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR11(value);
}

void Sound::SetNR12(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR12(value);
}

void Sound::SetNR13(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR13(value);
}

void Sound::SetNR14(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound1->SetNR14(value);
}


void Sound::SetNR21(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound2->SetNR21(value);
}

void Sound::SetNR22(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound2->SetNR22(value);
}

void Sound::SetNR23(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound2->SetNR23(value);
}

void Sound::SetNR24(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound2->SetNR24(value);
}


void Sound::SetNR30(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR30(value);
}

void Sound::SetNR31(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR31(value);
}

void Sound::SetNR32(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR32(value);
}

void Sound::SetNR33(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR33(value);
}

void Sound::SetNR34(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound3->SetNR34(value);
}


void Sound::SetNR41(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound4->SetNR41(value);
}

void Sound::SetNR42(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound4->SetNR42(value);
}

void Sound::SetNR43(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound4->SetNR43(value);
}

void Sound::SetNR44(u8 value)
{
	TRACE_REGISTER_WRITE

	m_sound4->SetNR44(value);
}


void Sound::SetNR50(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_hasPower == false)
		return;

	m_nr50 = value;
}

void Sound::SetNR51(u8 value)
{
	TRACE_REGISTER_WRITE

	if(m_hasPower == false)
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
		if(m_hasPower == false)
		{
			m_hasPower = true;

			m_sound1->PowerOn();
			m_sound2->PowerOn();
			m_sound3->PowerOn();
			m_sound4->PowerOn();
		}
	}
	else
	{
		if(m_hasPower == true)
		{
			m_sound1->PowerOff();
			m_sound2->PowerOff();
			m_sound3->PowerOff();
			m_sound4->PowerOff();

			m_frameSequencerPosition = 7;
		}

		SetNR50(0);
		SetNR51(0);
		m_nr52 = 0x70;

		m_hasPower = false;
	}
	
	m_nr52 = (value & 0x80) | 0x70 | (m_nr52 & 0x0f);
}


void Sound::MixSamples(float samples[4], float (&outSamples)[2])
{
	//Mix the samples

	int numSampleValues[2] = {0, 0};

	for(int outputChannel=0;outputChannel<2;outputChannel++)
	{
		//Combine the 4 component channels into a final output channel value
		for(int componentChannel=0;componentChannel<4;componentChannel++)
		{
			if(m_terminalOutputs[outputChannel][componentChannel])
			{
				outSamples[outputChannel] += samples[componentChannel];
				numSampleValues[outputChannel]++;
			}
		}

		//Play silence if there are no components assigned to a channel
		if(numSampleValues[outputChannel] == 0)
		{
			outSamples[outputChannel] = 0.f;
			numSampleValues[outputChannel] = 1;
		}

		//Divide the final value by the number of contributing components to reduce it to [-1,+1] range
		outSamples[outputChannel] /= (float)numSampleValues[outputChannel];

		//In 8-bits per sample mode, the final value needs to be in the [0,1] range instead of [-1,+1]
		if(BytesPerSample == 1)
		{
			outSamples[outputChannel] /= 2.f;
			outSamples[outputChannel] += 0.5f;
		}
	}
}