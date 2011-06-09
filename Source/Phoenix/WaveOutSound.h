#ifndef WAVEOUTSOUND_H
#define WAVEOUTSOUND_H

class Phoenix;
class Machine;

class WaveOutSound
{
public:

	WaveOutSound();
	~WaveOutSound();

	void Initialize(Phoenix* phoenix);
	void Shutdown();

	void SetMachine(Machine* machine);

	void SetMute(bool mute);

private:

	class WaveOutSound_Private* m_private;
};

#endif
