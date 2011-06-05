#include "soundGenerator.h"


SoundGenerator::SoundGenerator()
{
	m_hasPower = true;
}


void SoundGenerator::Initialize()
{
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
}

void SoundGenerator::TickEnvelope()
{
}


float SoundGenerator::GetSample()
{
	return 0.f;
}


void SoundGenerator::WriteLengthRegister(u8 value)
{
}

void SoundGenerator::WriteEnvelopeRegister(u8 value)
{
}
