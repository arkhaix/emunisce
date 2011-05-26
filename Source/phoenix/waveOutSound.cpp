#include "WaveOutSound.h"

#include "windows.h"

class WaveOutSound_Private
{
public:

	Phoenix* _Phoenix;

	Machine* _Machine;
};

void WaveOutSound::Initialize(Phoenix* phoenix)
{
	m_private = new WaveOutSound_Private();
	m_private->_Phoenix = phoenix;
}

void WaveOutSound::Shutdown()
{
	delete m_private;
}

void WaveOutSound::SetMachine(Machine* machine)
{
	m_private->_Machine = machine;
}
