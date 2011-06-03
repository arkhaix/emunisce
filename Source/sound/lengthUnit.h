#ifndef LENGTHUNIT_H
#define LENGTHUNIT_H

class LengthUnit
{
public:

	LengthUnit();

	void SetTicksPerSecond(double ticksPerSecond);
	void SetDecrementsPerSecond(double decrementsPerSecond);

	void SetMaxLength(int maxLength);
	void SetCurrentLength(int currentLength);
	void SetInverseLength(int inverseLength);

	int GetCurrentLength();

	void Enable();
	void Disable();
	bool IsEnabled();

	void Run(int ticks);


private:

	void UpdateTicksPerDecrement();

	double m_ticksPerSecond;
	double m_decrementsPerSecond;

	double m_ticksPerDecrement;
	double m_ticksUntilNextDecrement;

	int m_maxLength;
	int m_currentLength;

	bool m_enabled;
	bool m_pendingEnable;
};

#endif
