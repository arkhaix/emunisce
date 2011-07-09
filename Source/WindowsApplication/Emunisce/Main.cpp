/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "windows.h"

#include "../WindowsPlatform/Window.h"

#include "Emunisce.h"
#include "ConsoleDebugger.h"
#include "../GdiPlusRenderer/GdiPlusRenderer.h"
using namespace Emunisce;

EmunisceApplication* g_phoenix = NULL;

DWORD WINAPI ConsoleThread(LPVOID /*param*/)
{
	ConsoleDebugger* debugger = g_phoenix->GetDebugger();
	debugger->Run();

	return 0;
}

INT WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, INT /*iCmdShow*/)
{
	g_phoenix = new EmunisceApplication();

	HANDLE emulationThreadHandle = CreateThread(NULL, 0, ConsoleThread, NULL, 0, NULL);
	
	g_phoenix->RunWindow();	///<Blocks until shutdown is requested

	WaitForSingleObject(emulationThreadHandle, 1000);

	delete g_phoenix;

	return 0;
}

