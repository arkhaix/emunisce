#include "memory.h"

Memory* Memory::CreateFromFile(const char* filename)
{
	//Open file

	//Read header

	//Close the file

	//Instantiate appropriate MBC class from header info
	Memory* mbc = NULL;

	if(mbc == NULL)
		return NULL;

	//Have the MBC class load the file
	mbc->LoadFile(filename);

	return mbc;
}
