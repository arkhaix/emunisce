#ifndef PHOENIX_H
#define PHOENIX_H

class ConsoleDebugger;
class GdiPlusRenderer;
class KeyboardInput;
class WaveOutSound;

class Machine;

class Phoenix
{
public:

	Phoenix();
	~Phoenix();

	void NotifyMachineChanged(Machine* newMachine);

	Machine* GetMachine();

	bool ShutdownRequested();
	void RequestShutdown();

	ConsoleDebugger* GetDebugger();
	GdiPlusRenderer* GetRenderer();
	KeyboardInput* GetInput();
	WaveOutSound* GetSound();

private:

	class Phoenix_Private* m_private;
};

#endif
