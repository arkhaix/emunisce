#include "soundGenerator.h"

#include "../common/machine.h"

#include "channelController.h"
#include "sound.h"


SoundGenerator::SoundGenerator()
{
	m_hasPower = true;
}


void SoundGenerator::Initialize(ChannelController* channelController)
{
	m_channelController = channelController;
}

void SoundGenerator::SetMachine(Machine* machine)
{
	m_machine = machine;
}


void SoundGenerator::PowerOff()
{
	m_hasPower = false;
}

void SoundGenerator::PowerOn()
{
	m_hasPower = true;
}


void SoundGenerator::Run(int ticks)
{
}


void SoundGenerator::TickLength()
{
	if(m_lengthCounterEnabled == false)
		return;

	if(m_lengthCounterValue > 0)
	{
		m_lengthCounterValue--;

		if(m_lengthCounterValue == 0)
			m_channelController->DisableChannel();
	}
}

void SoundGenerator::TickEnvelope()
{
	if(m_envelopeVolume == 0 || m_envelopeVolume == 15)
		return;

	if(m_envelopeEnabled == false)
		return;

	m_envelopeTimer--;
	while(m_envelopeTimer <= 0)
	{
		m_envelopeTimer += m_envelopePeriod;

		if(m_envelopeVolumeIncreasing == true)
			m_envelopeVolume++;
		else
			m_envelopeVolume--;

		if(m_envelopeVolume == 0 || m_envelopeVolume == 15)
			m_envelopeEnabled = false;
	}
}


float SoundGenerator::GetSample()
{
	return 0.f;
}


void SoundGenerator::Trigger()
{
	if(m_dacEnabled == true)
	{
		m_channelController->EnableChannel();
	}

	if(m_lengthCounterValue == 0)
		m_lengthCounterValue = m_lengthCounterMaxValue;

	int frameSequencerPosition = m_machine->GetSound()->GetFrameSequencerPosition();

	if(frameSequencerPosition == 0 || frameSequencerPosition == 2 ||
		frameSequencerPosition == 4 || frameSequencerPosition == 6)
	{
		if(m_lengthCounterEnabled == true && m_lengthCounterValue == m_lengthCounterMaxValue)
		{
			m_lengthCounterValue--;
		}
	}
}

void SoundGenerator::WriteTriggerRegister(u8 value)
{
	if(value & 0x40)
		EnableLengthCounter();
	else
		DisableLengthCounter();

	if(value & 0x80)
		Trigger();
}


void SoundGenerator::EnableLengthCounter()
{
	int frameSequencerPosition = m_machine->GetSound()->GetFrameSequencerPosition();

	if(frameSequencerPosition == 0 || frameSequencerPosition == 2 ||
		frameSequencerPosition == 4 || frameSequencerPosition == 6)
	{
		if(m_lengthCounterEnabled == false && m_lengthCounterValue > 0)
		{
			m_lengthCounterValue--;

			if(m_lengthCounterValue == 0)
				m_channelController->DisableChannel();
		}
	}

	m_lengthCounterEnabled = true;
}

void SoundGenerator::DisableLengthCounter()
{
	m_lengthCounterEnabled = false;
}

void SoundGenerator::WriteLengthRegister(u8 value)
{
	m_lengthCounterValue = m_lengthCounterMaxValue - value;
}


void SoundGenerator::WriteEnvelopeRegister(u8 value)
{
	m_envelopePeriod = value & 0x07;

	if(m_envelopePeriod == 0)
		m_envelopeEnabled = false;
	else
		m_envelopeEnabled = true;

	if(value & 0x08)
		m_envelopeVolumeIncreasing = true;
	else
		m_envelopeVolumeIncreasing = false;

	m_envelopeInitialVolume = (value & 0xf0) >> 4;

	//Disable the DAC?
	if(m_envelopeInitialVolume == 0 && m_envelopeVolumeIncreasing == false)
	{
		m_channelController->DisableChannel();
		m_dacEnabled = false;
	}
	else
	{
		m_dacEnabled = true;
	}
}
