#ifndef WAVEOUTSOUND_H
#define WAVEOUTSOUND_H

class Phoenix;
class Machine;

class WaveOutSound
{
public:

	void Initialize(Phoenix* phoenix);
	void Shutdown();

	void SetMachine(Machine* machine);

private:

	class WaveOutSound_Private* m_private;
};

#endif
