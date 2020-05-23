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

#include "../gdi_plus_renderer/gdi_plus_renderer.h"
#include "../win32_common/window.h"
#include "console_debugger.h"
#include "emunisce.h"
using namespace emunisce;

EmunisceApplication* g_phoenix = nullptr;

DWORD WINAPI ConsoleThread(LPVOID /*param*/) {
	ConsoleDebugger* debugger = g_phoenix->GetDebugger();
	debugger->Run();

	return 0;
}

INT WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR /*lpCmdLine*/, INT /*iCmdShow*/) {
	g_phoenix = new EmunisceApplication();

	HANDLE emulationThreadHandle = CreateThread(nullptr, 0, ConsoleThread, nullptr, 0, nullptr);

	g_phoenix->RunWindow();  ///< Blocks until shutdown is requested

	WaitForSingleObject(emulationThreadHandle, 1000);

	delete g_phoenix;

	return 0;
}
