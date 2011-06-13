/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "windows.h"

#include "../WindowsPlatform/Window.h"

#include "../Machine/Machine.h"
#include "../Display/Display.h"

#include "Phoenix.h"
#include "ConsoleDebugger.h"
#include "../GdiPlusRenderer/GdiPlusRenderer.h"

Phoenix* g_phoenix = NULL;

DWORD WINAPI EmulationThread(LPVOID param)
{
	ConsoleDebugger* debugger = g_phoenix->GetDebugger();
	debugger->Run();

	return 0;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	g_phoenix = new Phoenix();

	HANDLE emulationThreadHandle = CreateThread(NULL, 0, EmulationThread, NULL, 0, NULL);
	
	int lastFrameRendered = -1;

	while(g_phoenix->ShutdownRequested() == false)
	{
		Machine* machine = g_phoenix->GetMachine();
		if(machine)
		{
			if(machine->GetDisplay()->GetScreenBufferCount() != lastFrameRendered)
			{
				HWND hwnd = (HWND)g_phoenix->GetWindow()->GetHandle();
				RECT clientRect;

				GetClientRect(hwnd, &clientRect);
				InvalidateRect(hwnd, &clientRect, true);
				UpdateWindow(hwnd);
			}
		}

		g_phoenix->GetWindow()->PumpMessages();

		Sleep(10);
	}

	WaitForSingleObject(emulationThreadHandle, 1000);

	delete g_phoenix;

	return 0;
}

