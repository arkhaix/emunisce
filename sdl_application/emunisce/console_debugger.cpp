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
#define EmulatedMachine_ToString 1

#include "console_debugger.h"

#include <algorithm>
#include <iostream>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include "emunisce.h"
#include "machine_includes.h"
#include "platform_includes.h"
#include "user_interface.h"

using namespace emunisce;

ConsoleDebugger::ConsoleDebugger() {
	m_machine = nullptr;
	m_cpu = nullptr;
	m_display = nullptr;
	m_memory = nullptr;

	m_muteSound = false;
	m_breakpointsEnabled = false;

	m_squareSynthesisMethod = SquareSynthesisMethod::LinearInterpolation;
	m_displayFilter = DisplayFilter::NoFilter;

	m_recordingInput = false;
	m_playingInput = false;
}

void ConsoleDebugger::Initialize(EmunisceApplication* application) {
	m_application = application;
	m_userInterface = application;
}

void ConsoleDebugger::Shutdown() {
}

void ConsoleDebugger::SetMachine(EmulatedMachine* machine) {
	m_machine = machine;

	m_cpu = nullptr;
	m_display = machine->GetDisplay();
	m_memory = nullptr;

	machine->GetSound()->SetSquareSynthesisMethod(m_squareSynthesisMethod);
	m_userInterface->SetDisplayFilter(m_displayFilter);
}

void ConsoleDebugger::Run() {
	Help();

	while (m_application->ShutdownRequested() == false) {
		UpdateDisplay();
		FetchCommand();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
}

void ConsoleDebugger::Print(const char* text) {
	printf("%s", text);
}

void ConsoleDebugger::Help() {
	m_application->ExecuteConsoleCommand("help");
}

void ConsoleDebugger::UpdateDisplay() {
	// Display update logic can be implemented here
}

void ConsoleDebugger::FetchCommand() {
	std::string line = "";
	printf("\n> ");

	line = FetchLine();

	std::vector<std::string> splitLine = SplitCommand(line);
	std::string commandText = "";
	if (!splitLine.empty()) {
		commandText = splitLine[0];
	}

	bool result = m_application->ExecuteConsoleCommand(line.c_str());

	if (result == false) {
		printf("Unrecognized command. Use 'help' for a list of valid commands.\n");
	}
}

std::string ConsoleDebugger::FetchLine() {
	std::string line;
	std::getline(std::cin, line);
	return line;
}

std::vector<std::string> ConsoleDebugger::SplitCommand(std::string command) {
	std::vector<std::string> result;

	std::regex splitRegex("\\s+");
	std::sregex_token_iterator iter(command.begin(), command.end(), splitRegex, -1);
	std::sregex_token_iterator end;

	for (; iter != end; ++iter) {
		std::string token = *iter;
		if (!token.empty()) {
			result.push_back(token);
		}
	}

	return result;
}
