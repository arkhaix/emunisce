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

#include <SDL3/SDL.h>

#include <thread>

#include "console_debugger.h"
#include "emunisce.h"

using namespace emunisce;

EmunisceApplication* g_application = nullptr;

void ConsoleThread() {
	ConsoleDebugger* debugger = g_application->GetDebugger();
	debugger->Run();
}

int main(int argc, char* argv[]) {
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
		SDL_Log("SDL_Init failed: %s", SDL_GetError());
		return 1;
	}

	g_application = new EmunisceApplication();

	// Load ROM from command line if provided
	if (argc > 1) {
		SDL_Log("Loading ROM from command line: %s", argv[1]);
		if (g_application->LoadRom(argv[1])) {
			SDL_Log("ROM loaded successfully");
			g_application->Run();  // Auto-start emulation
		}
		else {
			SDL_Log("Failed to load ROM: %s", argv[1]);
		}
	}

	std::thread consoleThread(ConsoleThread);
	consoleThread.detach();

	g_application->RunMainLoop();

	delete g_application;

	SDL_Quit();

	return 0;
}
