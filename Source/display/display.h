#ifndef DISPLAY_H
#define DISPLAY_H

#include "../common/types.h"

class Display
{
public:

	void SetMachine(Machine* machine);
	void Initialize();
	void Reset();

private:

	Memory* m_memory;
};

#endif
