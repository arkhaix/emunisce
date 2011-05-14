#ifndef TYPES_H
#define TYPES_H

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;

#ifndef NULL
#define NULL 0
#endif


class CPU;
class Memory;


namespace MachineType
{
	typedef int Type;

	enum
	{
		GameBoy,
		GameBoyColor,
		SuperGameBoy,

		NumMachineTypes
	};

	static const char* ToString[] =
	{
		"GameBoy",
		"GameBoyColor",
		"SuperGameBoy",

		"NumMachineTypes"
	};
}

struct Machine
{
	MachineType::Type _MachineType;

	CPU* _CPU;
	Memory* _Memory;
};

#endif
