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
