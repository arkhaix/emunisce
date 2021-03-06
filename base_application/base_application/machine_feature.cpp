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
#include "machine_feature.h"
using namespace emunisce;

// MachineFeatures

MachineFeature::MachineFeature() {
	m_application = nullptr;

	m_hasFocus = false;

	m_wrappedMachine = nullptr;
	m_isWrappingComponent = false;

	m_wrappedDisplay = nullptr;
	m_wrappedInput = nullptr;
	m_wrappedMemory = nullptr;
	m_wrappedProcessor = nullptr;
	m_wrappedSound = nullptr;

	m_featureExecution = nullptr;

	m_featureDisplay = nullptr;
	m_featureInput = nullptr;
	m_featureMemory = nullptr;
	m_featureProcessor = nullptr;
	m_featureSound = nullptr;

	m_defaultScreenResolution.width = 640;
	m_defaultScreenResolution.height = 480;

	for (int y = 0; y < m_defaultScreenBuffer.GetHeight(); y++) {
		for (int x = 0; x < m_defaultScreenBuffer.GetWidth(); x++) {
			m_defaultScreenBuffer.SetPixel(x, y, (DisplayPixel)0);
		}
	}

	m_defaultAudioBuffer.NumSamples = m_defaultAudioBuffer.BufferSizeSamples;
	for (unsigned int i = 0; i < m_defaultAudioBuffer.NumSamples; i++) {
		m_defaultAudioBuffer.Samples[0][i] = SilentSample;
		m_defaultAudioBuffer.Samples[1][i] = SilentSample;
	}
}

MachineFeature::~MachineFeature() {
}

void MachineFeature::SetApplication(BaseApplication* application) {
	m_application = application;
}

void MachineFeature::SetComponentMachine(EmulatedMachine* componentMachine) {
	m_wrappedMachine = componentMachine;
	m_isWrappingComponent = true;

	if (m_wrappedMachine != nullptr) {
		m_wrappedDisplay = componentMachine->GetDisplay();
		m_wrappedInput = componentMachine->GetInput();
		m_wrappedMemory = componentMachine->GetMemory();
		m_wrappedProcessor = componentMachine->GetProcessor();
		m_wrappedSound = componentMachine->GetSound();
	}
	else {
		m_wrappedDisplay = nullptr;
		m_wrappedInput = nullptr;
		m_wrappedMemory = nullptr;
		m_wrappedProcessor = nullptr;
		m_wrappedSound = nullptr;
	}
}

void MachineFeature::SetEmulatedMachine(EmulatedMachine* wrappedMachine) {
	if (m_isWrappingComponent == true && m_wrappedMachine != nullptr) {
		MachineFeature* componentMachine = dynamic_cast<MachineFeature*>(m_wrappedMachine);
		if (componentMachine != nullptr) {
			componentMachine->SetEmulatedMachine(wrappedMachine);
			return;
		}
	}

	// This point only reached if we're not wrapping a component

	m_wrappedMachine = wrappedMachine;

	if (m_wrappedMachine != nullptr) {
		m_wrappedDisplay = wrappedMachine->GetDisplay();
		m_wrappedInput = wrappedMachine->GetInput();
		m_wrappedMemory = wrappedMachine->GetMemory();
		m_wrappedProcessor = wrappedMachine->GetProcessor();
		m_wrappedSound = wrappedMachine->GetSound();
	}
	else {
		m_wrappedDisplay = nullptr;
		m_wrappedInput = nullptr;
		m_wrappedMemory = nullptr;
		m_wrappedProcessor = nullptr;
		m_wrappedSound = nullptr;
	}
}

void MachineFeature::SetFocus(bool hasFocus) {
	m_hasFocus = hasFocus;
}

// EmulatedMachine

// Machine type
Machine::Type MachineFeature::GetType() {
	if (m_wrappedMachine == nullptr) {
		return Machine::NoMachine;
	}

	return m_wrappedMachine->GetType();
}

const char* MachineFeature::GetRomTitle() {
	if (m_wrappedMachine == nullptr) {
		return nullptr;
	}

	return m_wrappedMachine->GetRomTitle();
}

// Application interface
void MachineFeature::SetApplicationInterface(MachineToApplication* applicationInterface) {
	if (m_wrappedMachine == nullptr) {
		return;
	}

	m_wrappedMachine->SetApplicationInterface(applicationInterface);
}

void MachineFeature::AddApplicationEvent(ApplicationEvent& applicationEvent, bool relativeFrameCount /*= true*/) {
	if (m_wrappedMachine == nullptr) {
		return;
	}

	m_wrappedMachine->AddApplicationEvent(applicationEvent, relativeFrameCount);
}

void MachineFeature::RemoveApplicationEvent(unsigned int eventId) {
	if (m_wrappedMachine == nullptr) {
		return;
	}

	m_wrappedMachine->RemoveApplicationEvent(eventId);
}

// Component access
EmulatedDisplay* MachineFeature::GetDisplay() {
	return this;
}

EmulatedInput* MachineFeature::GetInput() {
	return this;
}

EmulatedMemory* MachineFeature::GetMemory() {
	return this;
}

EmulatedProcessor* MachineFeature::GetProcessor() {
	return this;
}

EmulatedSound* MachineFeature::GetSound() {
	return this;
}

// Machine info
unsigned int MachineFeature::GetFrameCount() {
	if ((m_hasFocus || m_wrappedMachine == nullptr) && m_featureExecution != nullptr) {
		return m_featureExecution->GetFrameCount();
	}
	else if (m_wrappedMachine != nullptr) {
		return m_wrappedMachine->GetFrameCount();
	}

	return 0;
}

unsigned int MachineFeature::GetTickCount() {
	if ((m_hasFocus || m_wrappedMachine == nullptr) && m_featureExecution != nullptr) {
		return m_featureExecution->GetTickCount();
	}
	else if (m_wrappedMachine != nullptr) {
		return m_wrappedMachine->GetTickCount();
	}

	return 0;
}

unsigned int MachineFeature::GetTicksPerSecond() {
	if ((m_hasFocus || m_wrappedMachine == nullptr) && m_featureExecution != nullptr) {
		return m_featureExecution->GetTicksPerSecond();
	}
	else if (m_wrappedMachine != nullptr) {
		return m_wrappedMachine->GetTicksPerSecond();
	}

	return 1;
}

unsigned int MachineFeature::GetTicksUntilNextFrame() {
	if ((m_hasFocus || m_wrappedMachine == nullptr) && m_featureExecution != nullptr) {
		return m_featureExecution->GetTicksUntilNextFrame();
	}
	else if (m_wrappedMachine != nullptr) {
		return m_wrappedMachine->GetTicksUntilNextFrame();
	}

	return (unsigned int)-1;
}

// Execution
void MachineFeature::Step() {
	if ((m_hasFocus || m_wrappedMachine == nullptr) && m_featureExecution != nullptr) {
		m_featureExecution->Step();
	}
	else if (m_wrappedMachine != nullptr) {
		m_wrappedMachine->Step();
	}
}

void MachineFeature::RunToNextFrame() {
	if ((m_hasFocus || m_wrappedMachine == nullptr) && m_featureExecution != nullptr) {
		m_featureExecution->RunToNextFrame();
	}
	else if (m_wrappedMachine != nullptr) {
		m_wrappedMachine->RunToNextFrame();
	}
}

// Persistence
void MachineFeature::SaveState(Archive& archive) {
	if (m_wrappedMachine == nullptr) {
		return;
	}

	m_wrappedMachine->SaveState(archive);
}

void MachineFeature::LoadState(Archive& archive) {
	if (m_wrappedMachine == nullptr) {
		return;
	}

	m_wrappedMachine->LoadState(archive);
}

// Debugging
void MachineFeature::EnableBreakpoint(int address) {
	if (m_wrappedMachine == nullptr) {
		return;
	}

	m_wrappedMachine->EnableBreakpoint(address);
}

void MachineFeature::DisableBreakpoint(int address) {
	if (m_wrappedMachine == nullptr) {
		return;
	}

	m_wrappedMachine->DisableBreakpoint(address);
}

// IEmulatedDisplay

ScreenResolution MachineFeature::GetScreenResolution() {
	if ((m_hasFocus || m_wrappedDisplay == nullptr) && m_featureDisplay != nullptr) {
		return m_featureDisplay->GetScreenResolution();
	}

	if (m_wrappedDisplay != nullptr) {
		return m_wrappedDisplay->GetScreenResolution();
	}

	return m_defaultScreenResolution;
}

ScreenBuffer* MachineFeature::GetStableScreenBuffer() {
	if ((m_hasFocus || m_wrappedDisplay == nullptr) && m_featureDisplay != nullptr) {
		return m_featureDisplay->GetStableScreenBuffer();
	}

	if (m_wrappedDisplay != nullptr) {
		return m_wrappedDisplay->GetStableScreenBuffer();
	}

	return &m_defaultScreenBuffer;
}

int MachineFeature::GetScreenBufferCount() {
	if ((m_hasFocus || m_wrappedDisplay == nullptr) && m_featureDisplay != nullptr) {
		return m_featureDisplay->GetScreenBufferCount();
	}

	if (m_wrappedDisplay != nullptr) {
		return m_wrappedDisplay->GetScreenBufferCount();
	}

	return 0;
}

// IEmulatedInput

unsigned int MachineFeature::NumButtons() {
	unsigned int result = 0;

	if (m_featureInput != nullptr) {
		result += m_featureInput->NumButtons();
	}

	if (m_wrappedInput != nullptr) {
		result += m_wrappedInput->NumButtons();
	}

	return result;
}

const char* MachineFeature::GetButtonName(unsigned int index) {
	if (m_featureInput != nullptr) {
		if (index < m_featureInput->NumButtons()) {
			return m_featureInput->GetButtonName(index);
		}

		index -= m_featureInput->NumButtons();
	}

	if (m_wrappedInput != nullptr) {
		return m_wrappedInput->GetButtonName(index);
	}

	return nullptr;
}

void MachineFeature::ButtonDown(unsigned int index) {
	if (m_featureInput != nullptr) {
		if (index < m_featureInput->NumButtons()) {
			return m_featureInput->ButtonDown(index);
		}

		index -= m_featureInput->NumButtons();
	}

	if (m_wrappedInput != nullptr) {
		return m_wrappedInput->ButtonDown(index);
	}

	return;
}

void MachineFeature::ButtonUp(unsigned int index) {
	if (m_featureInput != nullptr) {
		if (index < m_featureInput->NumButtons()) {
			return m_featureInput->ButtonUp(index);
		}

		index -= m_featureInput->NumButtons();
	}

	if (m_wrappedInput != nullptr) {
		return m_wrappedInput->ButtonUp(index);
	}

	return;
}

bool MachineFeature::IsButtonDown(unsigned int index) {
	if (m_featureInput != nullptr) {
		if (index < m_featureInput->NumButtons()) {
			return m_featureInput->IsButtonDown(index);
		}

		index -= m_featureInput->NumButtons();
	}

	if (m_wrappedInput != nullptr) {
		return m_wrappedInput->IsButtonDown(index);
	}

	return false;
}

// IEmulatedMemory

// IEmulatedProcessor

// IEmulatedSound

AudioBuffer MachineFeature::GetStableAudioBuffer() {
	if ((m_hasFocus || m_wrappedSound == nullptr) && m_featureSound != nullptr) {
		return m_featureSound->GetStableAudioBuffer();
	}

	if (m_wrappedSound != nullptr) {
		return m_wrappedSound->GetStableAudioBuffer();
	}

	return m_defaultAudioBuffer;
}

int MachineFeature::GetAudioBufferCount() {
	if ((m_hasFocus || m_wrappedSound == nullptr) && m_featureSound != nullptr) {
		return m_featureSound->GetAudioBufferCount();
	}

	if (m_wrappedSound != nullptr) {
		return m_wrappedSound->GetAudioBufferCount();
	}

	return 0;
}

void MachineFeature::SetSquareSynthesisMethod(SquareSynthesisMethod::Type method) {
	if ((m_hasFocus || m_wrappedSound == nullptr) && m_featureSound != nullptr) {
		return m_featureSound->SetSquareSynthesisMethod(method);
	}

	if (m_wrappedSound != nullptr) {
		return m_wrappedSound->SetSquareSynthesisMethod(method);
	}

	return;
}
