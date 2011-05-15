#include "display.h"

//70224 t-states per frame (59.7fps)
//4560 t-states per v-blank (mode 01)

//80 (77-83) t-states per line in mode 10 (oam in use)
//172 (169-175) t-states per line in mode 11 (oam + vram in use)
//204 (201-207) t-states per line in mode 00 (h-blank)

void Display::SetMachine(Machine* machine)
{
	if(machine)
	{
		machine->_Display = this;

		m_memory = machine->_Memory;
	}
}

void Display::Initialize()
{
}

void Display::Reset()
{
}
