#include "LengthUnit.h"

#include "../Common/Machine.h"

#include "ChannelController.h"
#include "SoundGenerator.h"
#include "Sound.h"


LengthUnit::LengthUnit(SoundGenerator* soundGenerator)
{
	m_soundGenerator = soundGenerator;
}


void LengthUnit::SetMaxValue(int maxValue)
{
	m_maxValue = maxValue;
}

void LengthUnit::Tick()
{
	if(m_enabled == false)
		return;

	if(m_value > 0)
	{
		m_value--;

		if(m_value == 0)
			m_soundGenerator->m_channelController->DisableChannel();
	}
}

void LengthUnit::Trigger()
{
	if(m_value == 0)
		m_value = m_maxValue;

	int frameSequencerPosition = m_soundGenerator->m_machine->GetSound()->GetFrameSequencerPosition();

	if(frameSequencerPosition == 0 || frameSequencerPosition == 2 ||
		frameSequencerPosition == 4 || frameSequencerPosition == 6)
	{
		if(m_enabled == true && m_value == m_maxValue)
		{
			m_value--;
		}
	}
}


void LengthUnit::Enable()
{
	int frameSequencerPosition = m_soundGenerator->m_machine->GetSound()->GetFrameSequencerPosition();

	if(frameSequencerPosition == 0 || frameSequencerPosition == 2 ||
		frameSequencerPosition == 4 || frameSequencerPosition == 6)
	{
		if(m_enabled == false && m_value > 0)
		{
			m_value--;

			if(m_value == 0)
				m_soundGenerator->m_channelController->DisableChannel();
		}
	}

	m_enabled = true;
}

void LengthUnit::Disable()
{
	m_enabled = false;
}

void LengthUnit::WriteLengthRegister(u8 value)
{
	m_value = m_maxValue - value;
}
