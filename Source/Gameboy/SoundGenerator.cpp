/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "SoundGenerator.h"
using namespace Emunisce;

#include "GameboyIncludes.h"

#include "Serialization/SerializationIncludes.h"

#include "ChannelController.h"
#include "EnvelopeUnit.h"
#include "LengthUnit.h"
#include "Sound.h"


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

void SoundGenerator::SetMachine(Gameboy* machine)
{
	m_machine = machine;
}


void SoundGenerator::Serialize(Archive& archive)
{
	SerializeItem(archive, m_hasPower);
	SerializeItem(archive, m_dacEnabled);

	m_lengthUnit->Serialize(archive);

	if(m_envelopeUnit != NULL)
		m_envelopeUnit->Serialize(archive);
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
