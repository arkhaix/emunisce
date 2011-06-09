#include "EnvelopeUnit.h"

#include "ChannelController.h"
#include "SoundGenerator.h"


EnvelopeUnit::EnvelopeUnit(SoundGenerator* soundGenerator)
{
	m_soundGenerator = soundGenerator;

	m_enabled = false;

	m_volumeIncreasing = false;
	m_initialVolume = 0;
	m_currentVolume = 0;

	m_timerPeriod = 0;
	m_timerValue = 0;
}


void EnvelopeUnit::Tick()
{
	if(m_enabled == false)
		return;

	if(m_timerPeriod == 0)
		return;

	m_timerValue--;
	while(m_timerValue <= 0)
	{
		m_timerValue += m_timerPeriod;

		if(m_volumeIncreasing == true)
			m_currentVolume++;
		else
			m_currentVolume--;

		m_currentVolume &= 0x0f;

		if(m_currentVolume == 0 || m_currentVolume == 15)
			m_enabled = false;
	}
}

void EnvelopeUnit::Trigger()
{
	m_timerValue = m_timerPeriod;
	m_currentVolume = m_initialVolume;
	m_enabled = true;
}


void EnvelopeUnit::WriteEnvelopeRegister(u8 value)
{
	m_timerPeriod = value & 0x07;

	if(m_timerPeriod == 0)
		m_enabled = false;
	else
		m_enabled = true;

	if(value & 0x08)
		m_volumeIncreasing = true;
	else
		m_volumeIncreasing = false;

	m_initialVolume = (value & 0xf0) >> 4;

	//Disable the DAC?
	if(m_initialVolume == 0 && m_volumeIncreasing == false)
	{
		m_soundGenerator->m_channelController->DisableChannel();
		m_soundGenerator->m_dacEnabled = false;
	}
	else
	{
		m_soundGenerator->m_dacEnabled = true;
	}
}

float EnvelopeUnit::GetCurrentVolume()
{
	return (float)m_currentVolume / 15.f;
}
