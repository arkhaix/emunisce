#ifndef ROMONLY_H
#define ROMONLY_H

#include "memory.h"

class RomOnly : public Memory
{
public:

	virtual void Initialize();
	virtual void Reset();

	virtual u8 Read8(u16 address);
	virtual u16 Read16(u16 address);

	virtual void Write8(u16 address, u8 value);
	virtual void Write16(u16 address, u16 value);

protected:

	virtual bool LoadFile(const char* filename);
};

#endif
