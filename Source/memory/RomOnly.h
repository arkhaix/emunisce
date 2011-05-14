#ifndef ROMONLY_H
#define ROMONLY_H

#include "memory.h"

class RomOnly : public Memory
{
public:

	virtual void Initialize();
	virtual void Reset();

protected:

	virtual bool LoadFile(const char* filename);

	u8 m_cartRomData[0x8000];	//0x0000 - 0x7fff = cart rom
	u8 m_cartRamData[0x2000];	//0xa000 - 0xbfff = cart ram //todo: Not all carts have this, but I don't know how to detect that.
};

#endif
