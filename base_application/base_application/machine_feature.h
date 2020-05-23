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
#ifndef MACHINEFEATURE_H
#define MACHINEFEATURE_H

#include "machine_includes.h"

namespace emunisce {

class BaseApplication;

class IExecutableFeature {
public:
	// Machine info
	virtual unsigned int GetFrameCount() = 0;
	virtual unsigned int GetTickCount() = 0;
	virtual unsigned int GetTicksPerSecond() = 0;
	virtual unsigned int GetTicksUntilNextFrame() = 0;

	// Execution
	virtual void Step() = 0;
	virtual void RunToNextFrame() = 0;
};

class MachineFeature : public EmulatedMachine,
					   public EmulatedDisplay,
					   public EmulatedInput,
					   public EmulatedMemory,
					   public EmulatedProcessor,
					   public EmulatedSound {
public:
	// MachineFeature

	MachineFeature();
	virtual ~MachineFeature();

	virtual void SetApplication(BaseApplication* application);

	virtual void SetComponentMachine(EmulatedMachine* componentMachine);
	virtual void SetEmulatedMachine(EmulatedMachine* emulatedMachine);

	virtual void SetFocus(bool hasFocus);

	// EmulatedMachine

	// Machine type
	Machine::Type GetType() override;
	const char* GetRomTitle() override;

	// Application interface
	void SetApplicationInterface(MachineToApplication* applicationInterface) override;
	void AddApplicationEvent(ApplicationEvent& applicationEvent, bool relativeFrameCount /*= true*/) override;
	void RemoveApplicationEvent(unsigned int eventId) override;

	// Component access
	EmulatedDisplay* GetDisplay() override;
	EmulatedInput* GetInput() override;
	EmulatedMemory* GetMemory() override;
	EmulatedProcessor* GetProcessor() override;
	EmulatedSound* GetSound() override;

	// Machine info
	unsigned int GetFrameCount() override;
	unsigned int GetTickCount() override;
	unsigned int GetTicksPerSecond() override;
	unsigned int GetTicksUntilNextFrame() override;

	// Execution
	void Step() override;
	void RunToNextFrame() override;

	// Persistence
	void SaveState(Archive& archive) override;
	void LoadState(Archive& archive) override;

	// Debugging
	void EnableBreakpoint(int address) override;
	void DisableBreakpoint(int address) override;

	// IEmulatedDisplay

	ScreenResolution GetScreenResolution() override;
	ScreenBuffer* GetStableScreenBuffer() override;
	int GetScreenBufferCount() override;

	// IEmulatedInput

	unsigned int NumButtons() override;
	const char* GetButtonName(unsigned int index) override;

	void ButtonDown(unsigned int index) override;
	void ButtonUp(unsigned int index) override;

	bool IsButtonDown(unsigned int index) override;

	// IEmulatedMemory

	// IEmulatedProcessor

	// IEmulatedSound

	AudioBuffer GetStableAudioBuffer() override;
	int GetAudioBufferCount() override;

	void SetSquareSynthesisMethod(SquareSynthesisMethod::Type method) override;

protected:
	BaseApplication* m_application;

	bool m_hasFocus;

	EmulatedMachine* m_wrappedMachine;
	bool m_isWrappingComponent;

	EmulatedDisplay* m_wrappedDisplay;
	EmulatedInput* m_wrappedInput;
	EmulatedMemory* m_wrappedMemory;
	EmulatedProcessor* m_wrappedProcessor;
	EmulatedSound* m_wrappedSound;

	IExecutableFeature* m_featureExecution;

	EmulatedDisplay* m_featureDisplay;
	EmulatedInput* m_featureInput;
	EmulatedMemory* m_featureMemory;
	EmulatedProcessor* m_featureProcessor;
	EmulatedSound* m_featureSound;

	ScreenResolution m_defaultScreenResolution;
	TScreenBuffer<640, 480> m_defaultScreenBuffer;
	AudioBuffer m_defaultAudioBuffer;
};

}  // namespace emunisce

#endif
