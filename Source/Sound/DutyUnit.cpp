/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "DutyUnit.h"


DutyUnit::DutyUnit()
{
	m_timerPeriod = 0;
	m_timerValue = 0;

	m_dutyPosition = 0;
	m_dutyMode = 0;

	int dutyTable[4][8] =
	{
		{0,0,0,0,0,0,0,1},
		{1,0,0,0,0,0,0,1},
		{1,0,0,0,0,1,1,1},
		{0,1,1,1,1,1,1,0}
	};

	for(int i=0;i<4;i++)
		for(int j=0;j<8;j++)
			m_dutyTable[i][j] = dutyTable[i][j];

	m_useFancyStuff = false;
	m_hasTransitioned = false;
	m_hitNyquist = false;
	m_ticksSinceLastSample = 0;
	m_sumSinceLastSample = 0;
}


void DutyUnit::Run(int ticks)
{
	if(m_timerPeriod == 0)
		return;

	if(m_useFancyStuff == false)
	{
		m_timerValue -= ticks;
		while(m_timerValue <= 0)
		{
			m_timerValue += m_timerPeriod;

			m_dutyPosition++;
			if(m_dutyPosition > 7)
				m_dutyPosition = 0;
		}

		return;
	}

	if(m_hitNyquist == true)
		return;

	m_ticksSinceLastSample += ticks;

	if(m_timerValue > ticks)
	{
		m_timerValue -= ticks;

		int sampleValue = 1;
		if(m_dutyTable[ m_dutyMode ][ m_dutyPosition ] == 0)
			sampleValue = -1;

		m_sumSinceLastSample += sampleValue * ticks;
		return;
	}

	if(m_hasTransitioned == true)
	{
		m_hitNyquist = true;
		return;
	}

	m_hasTransitioned = true;

	int sampleValue = 1;
	if(m_dutyTable[ m_dutyMode ][ m_dutyPosition ] == 0)
		sampleValue = -1;

	m_sumSinceLastSample += sampleValue * m_timerValue;

	m_dutyPosition++;
	if(m_dutyPosition > 7)
		m_dutyPosition = 0;

	ticks -= m_timerValue;
	m_timerValue = m_timerPeriod;

	return Run(ticks);
}

void DutyUnit::Trigger()
{
	m_dutyPosition = 0;
	m_timerValue = m_timerPeriod;
}


void DutyUnit::SetFrequency(int frequency)
{
	m_timerPeriod = (2048 - frequency) * 4;
}

void DutyUnit::WriteDutyRegister(u8 value)
{
	m_dutyMode = (value & 0xc0) >> 6;
}


float DutyUnit::GetSample()
{
	if(m_useFancyStuff == false)
	{
		if(m_dutyTable[ m_dutyMode ][ m_dutyPosition ] == 0)
			return -1.f;

		return 1.f;
	}

	float result = 0.f;
	
	if(m_ticksSinceLastSample > 0 && m_hitNyquist == false)
		result = (float)m_sumSinceLastSample / (float)m_ticksSinceLastSample;

	m_hasTransitioned = false;
	m_hitNyquist = false;
	m_ticksSinceLastSample = 0;
	m_sumSinceLastSample = 0;

	return result;
}


void DutyUnit::SetUseFancyStuff(bool fancyStuff)
{
	m_useFancyStuff = fancyStuff;
}
