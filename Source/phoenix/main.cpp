
#include <iostream>
using namespace std;

#include "../cpu/cpu.h"
#include "../memory/memory.h"

int main(void)
{
	Memory::Initialize();

	CPU cpu;

	cpu.af = 0x0102;

	printf("%02x%02x\n", cpu.a, cpu.f);
	printf("%04x\n", cpu.af);

	system("pause");
	return 0;
}
