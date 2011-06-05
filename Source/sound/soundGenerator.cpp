#include "soundGenerator.h"

#include "../common/machine.h"

#include "channelController.h"
#include "lengthUnit.h"
#include "sound.h"


SoundGenerator::SoundGenerator()
{
	m_lengthUnit = new LengthUnit(this);
	m_hasPower = true;
}

SoundGenerator::~SoundGenerator()
{
	delete m_lengthUnit;
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
	m_lengthUnit->Tick();
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

	m_lengthUnit->Trigger();
}

void SoundGenerator::WriteTriggerRegister(u8 value)
{
	if(value & 0x40)
		m_lengthUnit->Enable();
	else
		m_lengthUnit->Disable();

	if(value & 0x80)
		Trigger();
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
