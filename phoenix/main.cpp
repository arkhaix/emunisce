
#include <iostream>
using namespace std;

#include "../cpu/cpu.h"
#include "../memory/memory.h"

int main(void)
{
	Memory::Initialize();
	CPU::Initialize();

	CPU::af = 0x0102;

	printf("%02x%02x\n", CPU::a, CPU::f);
	printf("%04x\n", CPU::af);

	system("pause");
	return 0;
}
