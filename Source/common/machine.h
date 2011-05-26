#ifndef MACHINE_H
#define MACHINE_H

class Cpu;
class Memory;
class Display;
class Input;
class Sound;

namespace MachineType
{
	typedef int Type;

	enum
	{
		GameBoy,
		GameBoyColor,

		NumMachineTypes
	};

	static const char* ToString[] =
	{
		"GameBoy",
		"GameBoyColor",

		"NumMachineTypes"
	};
}

class Machine
{
public:

	//Creation
	static Machine* Create(const char* filename);
	static void Release(Machine* machine);

	//Information
	MachineType::Type GetMachineType();
	unsigned int GetFrameCount();
	unsigned int GetTicksPerSecond();

	//Components
	Cpu* GetCpu();
	Memory* GetMemory();
	Display* GetDisplay();
	Input* GetInput();
	Sound* GetSound();

	//Execution
	void Step();
	void RunOneFrame();
	void Run();
	void Stop();

	//Persistence
	bool SaveState(const char* filename);
	bool LoadState(const char* filename);

	//Debugging
	void EnableBreakpoint(int address);
	void DisableBreakpoint(int address);

protected:

	Machine();
	void Initialize();

	MachineType::Type m_machineType;

	Cpu* m_cpu;
	Memory* m_memory;
	Display* m_display;
	Input* m_input;
	Sound* m_sound;

	unsigned int m_frameCount;

	unsigned int m_ticksPerSecond;
	unsigned int m_ticksPerFrame;
	int m_frameTicksRemaining;
};

#endif
