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
}


float SoundGenerator::GetSample()
{
	return 0.f;
}


void SoundGenerator::Trigger()
{
	m_channelController->EnableChannel();

	if(m_lengthCounterValue == 0)
		m_lengthCounterValue = m_lengthCounterMaxValue;
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
}
