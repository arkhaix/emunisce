#ifndef LENGTHUNIT_H
#define LENGTHUNIT_H

#include "../common/types.h"

class SoundGenerator;


class LengthUnit
{
public:

	LengthUnit(SoundGenerator* soundGenerator);

	void SetMaxValue(int maxValue);

	void Tick();
	void Trigger();

	void Enable();
	void Disable();
	void WriteLengthRegister(u8 value);


private:

	SoundGenerator* m_soundGenerator;

	bool m_enabled;

	int m_value;
	int m_maxValue;
};

#endif
