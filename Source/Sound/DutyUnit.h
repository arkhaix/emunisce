#ifndef DUTYUNIT_H
#define DUTYUNIT_H

#include "../Common/Types.h"

class DutyUnit
{
public:

	DutyUnit();

	void Run(int ticks);
	void Trigger();

	void SetFrequency(int frequency);
	void WriteDutyRegister(u8 value);

	float GetSample();

private:

	int m_timerPeriod;
	int m_timerValue;

	int m_dutyPosition;
	int m_dutyMode;
	int m_dutyTable[4][8];
};

#endif
