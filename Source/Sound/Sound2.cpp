#include "Sound2.h"

#include "../Common/Machine.h"
#include "../Memory/Memory.h"

#include "DutyUnit.h"
#include "EnvelopeUnit.h"
#include "LengthUnit.h"


Sound2::Sound2()
{
	m_machine = NULL;

	m_dutyUnit = new DutyUnit();
	m_envelopeUnit = new EnvelopeUnit(this);

	m_lengthUnit->SetMaxValue(64);

	m_frequency = 0;
}

Sound2::~Sound2()
{
	delete m_envelopeUnit;
	delete m_dutyUnit;
}


//Sound component

void Sound2::Initialize(ChannelController* channelController)
{
	SoundGenerator::Initialize(channelController);

	m_nr20 = 0xff;	///<inaccessible
	SetNR21(0x3f);
	SetNR22(0x00);
	SetNR23(0xff);
	SetNR24(0xbf);
}

void Sound2::SetMachine(Machine* machine)
{
	SoundGenerator::SetMachine(machine);

	Memory* memory = machine->GetMemory();

	memory->SetRegisterLocation(0x15, &m_nr20, false);
	memory->SetRegisterLocation(0x16, &m_nr21, false);
	memory->SetRegisterLocation(0x17, &m_nr22, false);
	memory->SetRegisterLocation(0x18, &m_nr23, false);
	memory->SetRegisterLocation(0x19, &m_nr24, false);
}


//Sound generation

void Sound2::PowerOff()
{
	SetNR21(0);
	SetNR22(0);
	SetNR23(0);
	SetNR24(0);

	SoundGenerator::PowerOff();
}

void Sound2::PowerOn()
{
	SoundGenerator::PowerOn();
}


void Sound2::Run(int ticks)
{
	SoundGenerator::Run(ticks);

	m_dutyUnit->Run(ticks);
}


void Sound2::TickEnvelope()
{
	m_envelopeUnit->Tick();
}


float Sound2::GetSample()
{
	float sample = m_dutyUnit->GetSample();

	sample *= m_envelopeUnit->GetCurrentVolume();

	return sample;
}


//Registers

void Sound2::SetNR21(u8 value)
{
	//DMG allows writing this even when the power is off
	//todo: CGB does not

	if(m_hasPower == true)
	{
		m_dutyUnit->WriteDutyRegister(value & 0xc0);
		m_nr21 = value & 0xc0;
	}

	m_lengthUnit->WriteLengthRegister(value & 0x3f);

	m_nr21 |= 0x3f;
}

void Sound2::SetNR22(u8 value)
{
	if(m_hasPower == false)
		return;

	m_envelopeUnit->WriteEnvelopeRegister(value);

	m_nr22 = value;
}

void Sound2::SetNR23(u8 value)
{
	if(m_hasPower == false)
		return;

	m_frequency &= ~(0xff);
	m_frequency |= value;
	m_dutyUnit->SetFrequency(m_frequency);

	m_nr23 = 0xff;
}

void Sound2::SetNR24(u8 value)
{
	if(m_hasPower == false)
		return;

	m_frequency &= ~(0x700);
	m_frequency |= ((value & 0x07) << 8);
	m_dutyUnit->SetFrequency(m_frequency);

	WriteTriggerRegister(value);

	m_nr24 = value & 0x40;
	m_nr24 |= 0xbf;
}


void Sound2::Trigger()
{
	SoundGenerator::Trigger();
	m_dutyUnit->Trigger();
	m_envelopeUnit->Trigger();
}
