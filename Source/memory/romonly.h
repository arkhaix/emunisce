#ifndef ROMONLY_H
#define ROMONLY_H

#include "memory.h"

class RomOnly : public Memory
{
public:

protected:

	virtual bool LoadFile(const char* filename);
};

#endif
