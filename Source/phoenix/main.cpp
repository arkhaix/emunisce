
#include <iostream>
using namespace std;

#include "../cpu/cpu.h"
#include "../memory/memory.h"

int main(void)
{
	Machine machine;
	machine._MachineType = MachineType::GameBoy;

	CPU cpu;
	cpu.SetMachine(&machine);
	cpu.Initialize();

	cpu.bc = 0x0102;

	printf("%02x%02x\n", cpu.b, cpu.c);
	printf("%04x\n", cpu.bc);

	system("pause");
	return 0;
}
