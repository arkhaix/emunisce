#include "lengthUnit.h"


LengthUnit::LengthUnit()
{
	m_ticksPerSecond = 1.0;
	m_decrementsPerSecond = 1.0;

	m_ticksPerDecrement = 1;
	m_ticksUntilNextDecrement = 1;

	m_maxLength = 0;
	m_currentLength = 0;
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

void LengthUnit::Run(int ticks)
{
	if(m_currentLength == 0)
		return;

	m_ticksUntilNextDecrement -= (double)ticks;
	while(m_ticksUntilNextDecrement <= 0.0+1e-5)
	{
		m_ticksUntilNextDecrement += m_ticksPerDecrement;

		m_currentLength--;
	}
}


void LengthUnit::UpdateTicksPerDecrement()
{
	m_ticksPerDecrement = m_ticksPerSecond / m_decrementsPerSecond;
	m_ticksUntilNextDecrement = m_ticksPerDecrement;
}
