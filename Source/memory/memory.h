#ifndef MEMORY_H
#define MEMORY_H

#include "../common/types.h"

class Memory
{
public:

	static void Initialize();
	static void Reset();

	static u8 Read8(u16 address);
	static u16 Read16(u16 address);

	static void Write8(u16 address, u8 value);
	static void Write16(u16 address, u16 value);
};

#endif
