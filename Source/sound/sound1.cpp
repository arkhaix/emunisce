#include "sound1.h"

#include "../common/machine.h"
#include "../memory/memory.h"

#include "channelController.h"
#include "envelopeUnit.h"
#include "lengthUnit.h"


Sound1::Sound1()
{
	m_machine = NULL;

	m_envelopeUnit = new EnvelopeUnit(this);

	m_lengthUnit->SetMaxValue(64);
}

Sound1::~Sound1()
{
	delete m_envelopeUnit;
}


//Sound component

void Sound1::Initialize(ChannelController* channelController)
{
	SoundGenerator::Initialize(channelController);

	m_frequency = 0;
	m_frequencyShadow = 0;

	m_frequencyTimerValue = 0;
	m_frequencyTimerPeriod = 0;

	m_dutyMode = 0;
	m_dutyPosition = 0;

	int dutyTable[4][8] =
	{
		{0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1},
		{1,0,0,0,0,1,1,1},
		{0,1,1,1,1,1,1,0}
	};

	for(int i=0;i<4;i++)
		for(int j=0;j<8;j++)
			m_dutyTable[i][j] = dutyTable[i][j];

	SetNR10(0x80);
	SetNR11(0x3f);
	SetNR12(0x00);
	SetNR13(0xff);
	SetNR14(0xbf);
}

void Sound1::SetMachine(Machine* machine)
{
	SoundGenerator::SetMachine(machine);

	Memory* memory = machine->GetMemory();

	memory->SetRegisterLocation(0x10, &m_nr10, false);
	memory->SetRegisterLocation(0x11, &m_nr11, false);
	memory->SetRegisterLocation(0x12, &m_nr12, false);
	memory->SetRegisterLocation(0x13, &m_nr13, false);
	memory->SetRegisterLocation(0x14, &m_nr14, false);
}


//Sound generation
void Sound1::PowerOff()
{
	SetNR10(0);
	SetNR11(0);
	SetNR12(0);
	SetNR13(0);
	SetNR14(0);

	SoundGenerator::PowerOff();
}

void Sound1::PowerOn()
{
	SoundGenerator::PowerOn();
}


void Sound1::Run(int ticks)
{
	m_frequencyTimerPeriod = (2048 - m_frequency) * 4;

	if(m_frequencyTimerPeriod > 0)
	{
		m_frequencyTimerValue -= ticks;
		while(m_frequencyTimerValue <= 0)
		{
			m_frequencyTimerValue += m_frequencyTimerPeriod;

			m_dutyPosition++;
			if(m_dutyPosition > 7)
				m_dutyPosition = 0;
		}
	}

	SoundGenerator::Run(ticks);
}


void Sound1::TickEnvelope()
{
	m_envelopeUnit->Tick();
}

void Sound1::TickSweep()
{
	if(m_sweepEnabled == false)
		return;

	if(m_sweepTimerPeriod == 0)
		return;

	m_sweepTimerValue--;
	if(m_sweepTimerValue > 0)
		return;

	m_sweepTimerValue += m_sweepTimerPeriod;

	m_frequencyShadow = m_frequency;

	int newFrequency = CalculateFrequency();
	if(newFrequency > 2047)
	{
		m_channelController->DisableChannel();
		return;
	}

	if(m_sweepShift == 0)
		return;

	if(newFrequency < 0)
		newFrequency = 0;

	m_frequency = newFrequency;
	m_frequencyShadow = newFrequency;

	newFrequency = CalculateFrequency();
	if(newFrequency > 2047)
	{
		m_channelController->DisableChannel();
		return;
	}
}


float Sound1::GetSample()
{
	float sample = 1.f;
	if(m_dutyTable[ m_dutyMode ][ m_dutyPosition ] == 0)
		sample = -1.f;

	sample *= m_envelopeUnit->GetCurrentVolume();

	return sample;
}


//Registers

void Sound1::SetNR10(u8 value)
{
	if(m_hasPower == false)
		return;

	WriteSweepRegister(value);

	m_nr10 = value & 0x7f;
	m_nr10 |= 0x80;
}

void Sound1::SetNR11(u8 value)
{
	//DMG allows writing length even when the power is off
	//todo: CGB does not

	if(m_hasPower == true)
	{
		m_dutyMode = (value & 0xc0) >> 6;
		m_nr11 = value & 0xc0;
	}

	m_lengthUnit->WriteLengthRegister(value & 0x3f);

	m_nr11 |= 0x3f;
}

void Sound1::SetNR12(u8 value)
{
	if(m_hasPower == false)
		return;

	m_envelopeUnit->WriteEnvelopeRegister(value);

	m_nr12 = value;
}

void Sound1::SetNR13(u8 value)
{
	if(m_hasPower == false)
		return;

	m_frequency &= ~(0xff);
	m_frequency |= value;

	m_nr13 = 0xff;
}

void Sound1::SetNR14(u8 value)
{
	if(m_hasPower == false)
		return;

	m_frequency &= ~(0x700);
	m_frequency |= ((value & 0x07) << 8);

	WriteTriggerRegister(value);

	m_nr14 = value & 0x40;
	m_nr14 |= 0xbf;
}


void Sound1::Trigger()
{
	SoundGenerator::Trigger();
	m_envelopeUnit->Trigger();
	TriggerSweep();
}

void Sound1::TriggerSweep()
{
	m_frequencyShadow = m_frequency;

	m_sweepTimerValue = m_sweepTimerPeriod;

	if(m_sweepTimerPeriod == 0 && m_sweepShift == 0)
		m_sweepEnabled = false;
	else
		m_sweepEnabled = true;

	if(m_sweepShift > 0)
	{
		int newFrequency = CalculateFrequency();
		if(newFrequency > 2047)
		{
			m_channelController->DisableChannel();
		}
	}
}

void Sound1::WriteSweepRegister(u8 value)
{
	m_sweepShift = value & 0x07;

	if(value & 0x08)
		m_sweepIncreasing = false;
	else
		m_sweepIncreasing = true;

	m_sweepTimerPeriod = (value & 0x70) >> 4;
}

int Sound1::CalculateFrequency()
{
	int delta = m_frequencyShadow >> m_sweepShift;

	int result = m_frequencyShadow;

	if(m_sweepIncreasing == true)
		result += delta;
	else
		result -= delta;

	return result;
}
