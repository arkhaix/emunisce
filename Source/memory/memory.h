#ifndef MEMORY_H
#define MEMORY_H

#include "../common/types.h"

class Memory
{
public:

	static Memory* CreateFromFile(Machine* machine, const char* filename);

	virtual void Initialize() = 0;
	virtual void Reset() = 0;

	virtual u8 Read8(u16 address) = 0;
	virtual u16 Read16(u16 address) = 0;

	virtual void Write8(u16 address, u8 value) = 0;
	virtual void Write16(u16 address, u16 value) = 0;

protected:

	virtual ~Memory();

	u8 ReadRegister(u16 address);
	void WriteRegister(u16 address, u8 value);

	virtual bool LoadFile(const char* filename) = 0;
};

#endif
