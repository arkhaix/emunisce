#include "windows.h"

#include <conio.h>

#include <iostream>
using namespace std;

#include "../cpu/cpu.h"
#include "../display/display.h"
#include "../memory/memory.h"

void render(Display* display)
{
	system("cls");
	SetCursorPos(0, 0);

	char palette[4] = {32, 177, 178, 219};
	ScreenBuffer screenBuffer = display->GetStableScreenBuffer();

	for(int r=0;r<144;r++)
	{
		for(int c=0;c<160;c++)
		{
			putchar(palette[ screenBuffer.Pixels[r*160 + c].Value ]);
		}
	}
}

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
	int ticksPerFrame = 4194304;
	int frameTime = ticksPerFrame;
	int numFrames = 0;

	while(true)
	{
		int ticks = cpu.Execute();
		display.Run(ticks);
		frameTime -= ticks;
		if(frameTime <= 0)
		{
			numFrames++;
			printf("%d\n", numFrames);
			frameTime += ticksPerFrame;

			if(_kbhit())
			{
				int ch = _getch();
				if(ch == 'q' || ch == 'x')
					break;

				if(ch == 'r')
					render(&display);
			}
		}
	}

	return 0;
}
