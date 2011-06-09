#include "Phoenix.h"

#include "../Common/Machine.h"

#include "ConsoleDebugger.h"
#include "GdiPlusRenderer.h"
#include "KeyboardInput.h"
#include "WaveOutSound.h"

class Phoenix_Private
{
public:

	Machine* _Machine;

	ConsoleDebugger* _Debugger;
	GdiPlusRenderer* _Renderer;
	KeyboardInput* _Input;
	WaveOutSound* _Sound;

	bool _ShutdownRequested;

	Phoenix_Private()
	{
		_ShutdownRequested = false;

		_Machine = NULL;

		_Debugger = new ConsoleDebugger();
		_Renderer = new GdiPlusRenderer();
		_Input = new KeyboardInput();
		_Sound = new WaveOutSound();
	}

	~Phoenix_Private()
	{
		_ShutdownRequested = true;

		_Sound->Shutdown();
		_Input->Shutdown();
		_Renderer->Shutdown();
		_Debugger->Shutdown();

		delete _Sound;
		delete _Input;
		delete _Renderer;
		delete _Debugger;

		if(_Machine)
		{
			Machine::Release(_Machine);
		}
	}
};

Phoenix::Phoenix()
{
	m_private = new Phoenix_Private();

	m_private->_Debugger->Initialize(this);
	m_private->_Renderer->Initialize(this);
	m_private->_Input->Initialize(this);
	m_private->_Sound->Initialize(this);
}

Phoenix::~Phoenix()
{
	delete m_private;
}

void Phoenix::NotifyMachineChanged(Machine* newMachine)
{
	m_private->_Machine = newMachine;

	m_private->_Debugger->SetMachine(newMachine);
	m_private->_Renderer->SetMachine(newMachine);
	m_private->_Input->SetMachine(newMachine);
	m_private->_Sound->SetMachine(newMachine);
}

bool Phoenix::ShutdownRequested()
{
	return m_private->_ShutdownRequested;
}

void Phoenix::RequestShutdown()
{
	m_private->_ShutdownRequested = true;
}

ConsoleDebugger* Phoenix::GetDebugger()
{
	return m_private->_Debugger;
}

GdiPlusRenderer* Phoenix::GetRenderer()
{
	return m_private->_Renderer;
}

KeyboardInput* Phoenix::GetInput()
{
	return m_private->_Input;
}

WaveOutSound* Phoenix::GetSound()
{
	return m_private->_Sound;
}
