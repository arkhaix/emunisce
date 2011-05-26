#ifndef PHOENIX_H
#define PHOENIX_H

class ConsoleDebugger;
class GdiPlusRenderer;
class KeyboardInput;

class Machine;

class Phoenix
{
public:

	Phoenix();
	~Phoenix();

	void NotifyMachineChanged(Machine* newMachine);

	bool ShutdownRequested();
	void RequestShutdown();

	ConsoleDebugger* GetDebugger();
	GdiPlusRenderer* GetRenderer();
	KeyboardInput* GetInput();

private:

	class Phoenix_Private* m_private;
};

#endif
