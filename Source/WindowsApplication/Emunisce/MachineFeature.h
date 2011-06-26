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

#include "MachineIncludes.h"


namespace Emunisce
{

class IExecutableFeature
{
public:

	//Machine info
	virtual unsigned int GetFrameCount() = 0;
	virtual unsigned int GetTicksPerSecond() = 0;
	virtual unsigned int GetTicksUntilNextFrame() = 0;

	//Execution
	virtual void Step() = 0;
	virtual void RunToNextFrame() = 0;
};

class MachineFeature : public IEmulatedMachine, public IEmulatedDisplay, public IEmulatedInput, public IEmulatedMemory, public IEmulatedProcessor, public IEmulatedSound
{
public:

	// MachineFeature

	MachineFeature();
	virtual ~MachineFeature();

	virtual void SetMachine(IEmulatedMachine* wrappedMachine);


	// IEmulatedMachine

	//Machine type
	virtual EmulatedMachine::Type GetType();
	virtual const char* GetRomTitle();
	
	//Application interface
	virtual void SetApplicationInterface(IMachineToApplication* applicationInterface);

	//Component access
	virtual IEmulatedDisplay* GetDisplay();
	virtual IEmulatedInput* GetInput();
	virtual IEmulatedMemory* GetMemory();
	virtual IEmulatedProcessor* GetProcessor();
	virtual IEmulatedSound* GetSound();

	//Machine info
	virtual unsigned int GetFrameCount();
	virtual unsigned int GetTicksPerSecond();
	virtual unsigned int GetTicksUntilNextFrame();

	//Execution
	virtual void Step();
	virtual void RunToNextFrame();

	//Persistence
	virtual void SaveState(Archive& archive);
	virtual void LoadState(Archive& archive);

	//Debugging
	virtual void EnableBreakpoint(int address);
	virtual void DisableBreakpoint(int address);


	// IEmulatedDisplay

	virtual ScreenResolution GetScreenResolution();
	virtual ScreenBuffer* GetStableScreenBuffer();
	virtual int GetScreenBufferCount();


	// IEmulatedInput

	virtual unsigned int NumButtons();
	virtual const char* GetButtonName(unsigned int index);

	virtual void ButtonDown(unsigned int index);
	virtual void ButtonUp(unsigned int index);


	// IEmulatedMemory

	// IEmulatedProcessor

	// IEmulatedSound

	virtual AudioBuffer GetStableAudioBuffer();
	virtual int GetAudioBufferCount();

	virtual void SetSquareSynthesisMethod(SquareSynthesisMethod::Type method);


protected:

	bool m_hasFocus;

	IEmulatedMachine* m_wrappedMachine;

	IEmulatedDisplay* m_wrappedDisplay;
	IEmulatedInput* m_wrappedInput;
	IEmulatedMemory* m_wrappedMemory;
	IEmulatedProcessor* m_wrappedProcessor;
	IEmulatedSound* m_wrappedSound;

	IExecutableFeature* m_featureExecution;

	IEmulatedDisplay* m_featureDisplay;
	IEmulatedInput* m_featureInput;
	IEmulatedMemory* m_featureMemory;
	IEmulatedProcessor* m_featureProcessor;
	IEmulatedSound* m_featureSound;

	ScreenResolution m_defaultScreenResolution;
	TScreenBuffer<640, 480> m_defaultScreenBuffer;
	AudioBuffer m_defaultAudioBuffer;
};

}	//namespace Emunisce

#endif
