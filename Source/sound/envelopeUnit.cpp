#include "envelopeUnit.h"

#include "channelController.h"
#include "soundGenerator.h"


EnvelopeUnit::EnvelopeUnit(SoundGenerator* soundGenerator)
{
	m_soundGenerator = soundGenerator;
}


void EnvelopeUnit::Tick()
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


void EnvelopeUnit::WriteEnvelopeRegister(u8 value)
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
		m_soundGenerator->m_channelController->DisableChannel();
		m_soundGenerator->m_dacEnabled = false;
	}
	else
	{
		m_soundGenerator->m_dacEnabled = true;
	}
}
