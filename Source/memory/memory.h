#ifndef MEMORY_H
#define MEMORY_H

#include "../common/types.h"

class Memory
{
public:

	static Memory* CreateFromFile(const char* filename);

	virtual void Initialize() = 0;
	virtual void Reset() = 0;

	virtual u8 Read8(u16 address) = 0;
	virtual u16 Read16(u16 address) = 0;

	virtual void Write8(u16 address, u8 value) = 0;
	virtual void Write16(u16 address, u16 value) = 0;

protected:

	virtual void LoadFile(const char* filename);
};

#endif
