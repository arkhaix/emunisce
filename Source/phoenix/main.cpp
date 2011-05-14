
#include <iostream>
using namespace std;

#include "../cpu/cpu.h"
#include "../memory/memory.h"

int main(void)
{
	Machine machine;
	machine._MachineType = MachineType::GameBoy;

	CPU cpu;

	Memory* memory = Memory::CreateFromFile("test.gb");
	if(memory == NULL)
	{
		printf("Unsupported memory controller\n");
		system("pause");
		return 1;
	}

	cpu.SetMachine(&machine);
	memory->SetMachine(&machine);

	cpu.Initialize();
	memory->Initialize();

	cpu.bc = 0x0102;

	printf("%02x%02x\n", cpu.b, cpu.c);
	printf("%04x\n", cpu.bc);

	system("pause");
	return 0;
}
