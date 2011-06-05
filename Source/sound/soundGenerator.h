#ifndef SOUNDGENERATOR_H
#define SOUNDGENERATOR_H

class SoundGenerator
{
public:

	SoundGenerator();

	virtual void Initialize();

	virtual void PowerOff();
	virtual void PowerOn();

	void Run(int ticks);

	void TickLength();
	void TickEnvelope();

	float GetSample();

protected:

	bool m_hasPower;
};

#endif
