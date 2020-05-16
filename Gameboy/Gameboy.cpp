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
#include "Gameboy.h"
using namespace Emunisce;

// Gameboy
#include "GameboyIncludes.h"

// Serialization
#include "Serialization/SerializationIncludes.h"

// IEmulatedMachine

// Machine type
EmulatedMachine::Type Gameboy::GetType() {
	return m_machineType;
}

const char* Gameboy::GetRomTitle() {
	return m_romTitle;
}

// Application interface
void Gameboy::SetApplicationInterface(IMachineToApplication* applicationInterface) {
	m_applicationInterface = applicationInterface;
}

void Gameboy::AddApplicationEvent(ApplicationEvent& applicationEvent, bool relativeFrameCount) {
	std::lock_guard<std::mutex> scopedLock(m_applicationEventsLock);

	if (relativeFrameCount == true) {
		ApplicationEvent eventCopy = applicationEvent;
		eventCopy.frameCount += m_frameCount;
		m_applicationEvents.push_back(eventCopy);
	} else {
		m_applicationEvents.push_back(applicationEvent);
	}

	m_nextApplicationEvent = min_element(m_applicationEvents.begin(), m_applicationEvents.end());

	m_applicationEventsPending = true;
}

void Gameboy::RemoveApplicationEvent(unsigned int eventId) {
	std::lock_guard<std::mutex> scopedLock(m_applicationEventsLock);

	for (auto iter = m_applicationEvents.begin(); iter != m_applicationEvents.end(); ++iter) {
		if (iter->eventId == eventId) {
			m_applicationEvents.erase(iter);
			break;
		}
	}

	m_nextApplicationEvent = min_element(m_applicationEvents.begin(), m_applicationEvents.end());

	if (m_applicationEvents.empty()) {
		m_applicationEventsPending = false;
	}
}

// Component access
IEmulatedDisplay* Gameboy::GetDisplay() {
	return m_display;
}

IEmulatedInput* Gameboy::GetInput() {
	return m_input;
}

IEmulatedMemory* Gameboy::GetMemory() {
	return m_memory;
}

IEmulatedProcessor* Gameboy::GetProcessor() {
	return m_cpu;
}

IEmulatedSound* Gameboy::GetSound() {
	return m_sound;
}

// Machine info
unsigned int Gameboy::GetFrameCount() {
	return m_frameCount;
}

unsigned int Gameboy::GetTickCount() {
	return m_ticksPerFrame - m_frameTicksRemaining;
}

unsigned int Gameboy::GetTicksPerSecond() {
	return m_ticksPerSecond;
}

unsigned int Gameboy::GetTicksUntilNextFrame() {
	return m_frameTicksRemaining;
}

// Execution
void Gameboy::Step() {
	InternalStep();
}

void Gameboy::RunToNextFrame() {
	unsigned int currentFrame = m_frameCount;
	while (m_frameCount == currentFrame) {
		InternalStep();
	}
}

void Gameboy::Run() {}

void Gameboy::Stop() {}

// Persistence
void Gameboy::SaveState(Archive& ar) {
	Serialize(ar);
}

void Gameboy::LoadState(Archive& ar) {
	Serialize(ar);
}

// Debugging
void Gameboy::EnableBreakpoint(int address) {}

void Gameboy::DisableBreakpoint(int address) {}

// Gameboy

// Creation
Gameboy* Gameboy::Create(const char* filename, EmulatedMachine::Type machineType) {
	Memory* memory = Memory::CreateFromFile(filename);
	if (memory == nullptr) {
		return nullptr;
	}

	if (machineType == EmulatedMachine::AutoSelect) {
		u8 cgbValue = memory->Read8(0x0143);

		if (cgbValue & 0x80) {
			machineType = EmulatedMachine::GameboyColor;
		} else {
			machineType = EmulatedMachine::Gameboy;
		}
	}

	if (machineType != EmulatedMachine::Gameboy && machineType != EmulatedMachine::GameboyColor) {
		return nullptr;
	}

	Gameboy* result = new Gameboy(memory, machineType);

	result->Initialize();

	return result;
}

void Gameboy::Release(Gameboy* machine) {
	if (machine != nullptr) {
		delete machine;
	}
}

// Application interface
IMachineToApplication* Gameboy::GetApplicationInterface() {
	return m_applicationInterface;
}

// Gameboy Components (non-functions, direct concrete types)
Cpu* Gameboy::GetGbCpu() {
	return m_cpu;
}

Memory* Gameboy::GetGbMemory() {
	return m_memory;
}

Display* Gameboy::GetGbDisplay() {
	return m_display;
}

Input* Gameboy::GetGbInput() {
	return m_input;
}

Sound* Gameboy::GetGbSound() {
	return m_sound;
}

// Execution
void Gameboy::RunDuringInstruction(unsigned int ticks) {
	if (m_executingInstruction == false) {
		return;
	}

	if (m_applicationEventsPending == true) {
		std::lock_guard<std::mutex> scopedLock(m_applicationEventsLock);

		if (m_applicationEvents.empty() == false && m_nextApplicationEvent != m_applicationEvents.end()) {
			ApplicationEvent currentTime;
			currentTime.frameCount = m_frameCount;
			currentTime.tickCount = (m_ticksPerFrame - m_frameTicksRemaining) + ticks;

			if ((*m_nextApplicationEvent) < currentTime) {
				m_applicationInterface->HandleApplicationEvent(m_nextApplicationEvent->eventId);
				m_applicationEvents.erase(m_nextApplicationEvent);
				m_nextApplicationEvent = min_element(m_applicationEvents.begin(), m_applicationEvents.end());

				if (m_applicationEvents.empty()) {
					m_applicationEventsPending = false;
				}
			}
		}
	}

	m_cpu->RunTimer(ticks);

	int slowTicks = ticks;  ///< ticks to run for components that don't do double-speed
	if (m_doubleSpeed == true) {
		// This naive integer divide is safe because all instruction times are even multiples of two
		// and so are all the sub-instruction tick calls, so ticks and ticksThisStep will always be even
		// prior to this divide.
		slowTicks /= 2;
	}

	m_memory->Run(slowTicks);

	if (m_cpu->IsStopped() == false) {
		m_display->Run(slowTicks);
	}

	m_sound->Run(slowTicks);

	m_subInstructionTicksSpent += ticks;  ///< Not slowTicks.  This is the number of double-speed ticks used because
										  ///< it's later subtracted from the instruction time in InternalStep.
}

bool Gameboy::IsDoubleSpeed() {
	return m_doubleSpeed;
}

void Gameboy::SetDoubleSpeed(bool doubleSpeed) {
	m_doubleSpeed = doubleSpeed;
}

// protected:

Gameboy::Gameboy(Memory* memory, EmulatedMachine::Type machineType) {
	m_machineType = machineType;

	m_applicationInterface = nullptr;

	m_memory = memory;
	m_cpu = new Cpu();
	m_display = new Display();
	m_input = new Input();
	m_sound = new Sound();

	for (char& c : m_romTitle) {
		c = 0;
	}

	m_frameCount = 0;

	m_ticksPerSecond = 4194304;
	m_ticksPerFrame = 69905;
	m_frameTicksRemaining = m_ticksPerFrame;

	m_executingInstruction = false;
	m_subInstructionTicksSpent = 0;

	m_doubleSpeed = false;

	m_nextApplicationEvent = m_applicationEvents.end();
	m_applicationEventsPending = false;
}

Gameboy::~Gameboy() {
	delete m_sound;
	delete m_input;
	delete m_display;
	delete m_cpu;
	delete m_memory;
}

void Gameboy::Initialize() {
	m_memory->SetMachine(this);
	m_cpu->SetMachine(this);
	m_display->SetMachine(this);
	m_input->SetMachine(this);
	m_sound->SetMachine(this);

	m_memory->Initialize();
	m_cpu->Initialize();
	m_display->Initialize();
	m_input->Initialize();
	m_sound->Initialize();

	// Get the rom title
	for (int i = 0; i < 11; i++) {
		char ch = m_memory->Read8(0x0134 + i);
		if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') || (ch == ' ')) {
			m_romTitle[i] = ch;
		} else {
			m_romTitle[i] = 0;
		}
	}
}

void Gameboy::InternalStep() {
	if (m_applicationEventsPending == true) {
		std::lock_guard<std::mutex> scopedLock(m_applicationEventsLock);

		if (m_applicationEvents.empty() == false && m_nextApplicationEvent != m_applicationEvents.end()) {
			ApplicationEvent currentTime;
			currentTime.frameCount = m_frameCount;
			currentTime.tickCount = m_ticksPerFrame - m_frameTicksRemaining;

			if ((*m_nextApplicationEvent) < currentTime) {
				m_applicationInterface->HandleApplicationEvent(m_nextApplicationEvent->eventId);
				m_applicationEvents.erase(m_nextApplicationEvent);
				m_nextApplicationEvent = min_element(m_applicationEvents.begin(), m_applicationEvents.end());

				if (m_applicationEvents.empty()) {
					m_applicationEventsPending = false;
				}
			}
		}
	}

	m_executingInstruction = true;
	unsigned int ticks = m_cpu->Step();
	m_executingInstruction = false;

	unsigned int ticksThisStep = ticks;

	// If the instruction spent more ticks than its total time (should never happen)
	// then roll over the spent ticks to the next instruction
	if (ticks < m_subInstructionTicksSpent) {
		m_subInstructionTicksSpent -= ticks;
		ticks = 0;
	}

	// Otherwise (normal case), just subtract the spent ticks from the total instruction time
	else {
		ticks -= m_subInstructionTicksSpent;
		m_subInstructionTicksSpent = 0;
	}

	m_cpu->RunTimer(ticks);

	int slowTicks = ticks;  ///< Number of ticks to run for components that don't do double-speed.
	if (m_doubleSpeed == true) {
		// This naive integer divide is safe because all instruction times are even multiples of two
		// and so are all the sub-instruction tick calls, so ticks and ticksThisStep will always be even
		// prior to this divide.
		slowTicks /= 2;
		ticksThisStep /= 2;
	}

	m_memory->Run(slowTicks);

	if (m_cpu->IsStopped() == false) {
		m_display->Run(slowTicks);
	}

	m_sound->Run(slowTicks);

	m_frameTicksRemaining -= ticksThisStep;  ///< This is halved during double-speed.
	if (m_frameTicksRemaining <= 0) {
		m_frameCount++;
		m_frameTicksRemaining += m_ticksPerFrame;
	}
}

void Gameboy::Serialize(Archive& archive) {
	SerializeItem(archive, m_frameCount);

	SerializeItem(archive, m_ticksPerSecond);
	SerializeItem(archive, m_ticksPerFrame);
	SerializeItem(archive, m_frameTicksRemaining);

	SerializeItem(archive, m_doubleSpeed);

	m_cpu->Serialize(archive);
	m_display->Serialize(archive);
	m_input->Serialize(archive);
	m_memory->Serialize(archive);
	m_sound->Serialize(archive);
}
