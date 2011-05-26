#ifndef SOUND_H
#define SOUND_H

#include "../common/types.h"

struct AudioBuffer
{
	u8 Samples[735];	///<44100Hz / 60fps
};

class Sound
{
public:

	Sound();
	~Sound();

	//Component
	void SetMachine(Machine* machine);
	void Initialize();

	void Run(int ticks);

	//External
	AudioBuffer GetStableAudioBuffer();

	//Registers

private:

	Machine* m_machine;

	AudioBuffer m_audioBuffer[2];
	AudioBuffer* m_activeAudioBuffer;
	AudioBuffer* m_stableAudioBuffer;
	void* m_audioBufferLock;

	//Registers
};

#endif
