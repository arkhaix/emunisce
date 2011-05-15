
#include <iostream>
using namespace std;

#include "../cpu/cpu.h"
#include "../display/display.h"
#include "../memory/memory.h"

int main(void)
{
	Machine machine;
	machine._MachineType = MachineType::GameBoy;

	CPU cpu;
	machine._CPU = &cpu;

	Display display;
	machine._Display = &display;

	Memory* memory = Memory::CreateFromFile("C:/hg/Phoenix/Roms/testh.gb");
	if(memory == NULL)
	{
		printf("Unsupported memory controller\n");
		system("pause");
		return 1;
	}
	machine._Memory = memory;

	cpu.SetMachine(&machine);
	display.SetMachine(&machine);
	memory->SetMachine(&machine);

	memory->Initialize();
	cpu.Initialize();
	display.Initialize();

	//4.194304 MHz
	//4194304 Hz

	cpu.bc = 0x0102;

	printf("%02x%02x\n", cpu.b, cpu.c);
	printf("%04x\n", cpu.bc);

	system("pause");
	return 0;
}
