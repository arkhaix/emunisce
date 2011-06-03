#include "lengthUnit.h"


LengthUnit::LengthUnit()
{
	m_ticksPerSecond = 1.0;
	m_decrementsPerSecond = 1.0;

	m_ticksPerDecrement = 1;
	m_ticksUntilNextDecrement = 1;

	m_maxLength = 0;
	m_currentLength = 0;

	m_enabled = true;
	m_pendingEnable = false;
}

void LengthUnit::SetTicksPerSecond(double ticksPerSecond)
{
	m_ticksPerSecond = ticksPerSecond;
	if(m_ticksPerSecond <= 0.0)
		m_ticksPerSecond = 1.0;

	UpdateTicksPerDecrement();
}

void LengthUnit::SetDecrementsPerSecond(double decrementsPerSecond)
{
	m_decrementsPerSecond = decrementsPerSecond;
	if(m_decrementsPerSecond <= 0.0)
		m_decrementsPerSecond = 1.0;

	UpdateTicksPerDecrement();
}

void LengthUnit::SetMaxLength(int maxLength)
{
	m_maxLength = maxLength;
}

void LengthUnit::SetCurrentLength(int currentLength)
{
	m_currentLength = currentLength;

	if(m_currentLength < 0)
		m_currentLength = 0;
	if(m_currentLength > m_maxLength)
		m_currentLength = m_maxLength;
}

void LengthUnit::SetInverseLength(int inverseLength)
{
	SetCurrentLength(m_maxLength - inverseLength);
}

int LengthUnit::GetCurrentLength()
{
	return m_currentLength;
}

void LengthUnit::Enable()
{
	//if(m_ticksUntilNextDecrement * 2.0 >= m_ticksPerDecrement)
		m_enabled = true;
	//else
	//	m_pendingEnable = true;
}

void LengthUnit::Disable()
{
	m_enabled = false;
	m_pendingEnable = false;
}

bool LengthUnit::IsEnabled()
{
	return m_enabled;
}

void LengthUnit::Run(int ticks)
{
	m_ticksUntilNextDecrement -= (double)ticks;
	while(m_ticksUntilNextDecrement <= 0.0+1e-5)
	{
		m_ticksUntilNextDecrement += m_ticksPerDecrement;

		if(m_enabled == true && m_currentLength > 0)
		{
			m_currentLength--;
		}
		else if(m_pendingEnable == true)
		{
			m_enabled = true;
			m_pendingEnable = false;
		}
	}
}


void LengthUnit::UpdateTicksPerDecrement()
{
	m_ticksPerDecrement = m_ticksPerSecond / m_decrementsPerSecond;
	m_ticksUntilNextDecrement = m_ticksPerDecrement;
}
