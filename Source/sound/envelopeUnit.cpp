#include "envelopeUnit.h"

#include "channelController.h"
#include "soundGenerator.h"


EnvelopeUnit::EnvelopeUnit(SoundGenerator* soundGenerator)
{
	m_soundGenerator = soundGenerator;
}


void EnvelopeUnit::Tick()
{
	if(m_currentVolume == 0 || m_currentVolume == 15)
		return;

	if(m_enabled == false)
		return;

	m_timerValue--;
	while(m_timerValue <= 0)
	{
		m_timerValue += m_timerPeriod;

		if(m_volumeIncreasing == true)
			m_currentVolume++;
		else
			m_currentVolume--;

		if(m_currentVolume == 0 || m_currentVolume == 15)
			m_enabled = false;
	}
}

void EnvelopeUnit::Trigger()
{
	m_timerValue = m_timerPeriod;
	m_currentVolume = m_initialVolume;
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
