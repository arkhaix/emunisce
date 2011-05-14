#include "memory.h"

Memory* Memory::CreateFromFile(Machine* machine, const char* filename)
{
	//Open file

	//Read header

	//Close the file

	//Instantiate appropriate MBC class from header info
	Memory* mbc = NULL;

	if(mbc == NULL)
		return NULL;

	//Have the MBC class load the file
	if(mbc->LoadFile(filename) == false)
	{
		delete mbc;
		return NULL;
	}

	return mbc;
}

Memory::~Memory()
{
}

u8 Memory::ReadRegister(u16 address)
{
	return 0;
}

void Memory::WriteRegister(u16 address, u8 value)
{
}
