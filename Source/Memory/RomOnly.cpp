#include "RomOnly.h"

#include <cstdlib>

#include <fstream>
using namespace std;


bool RomOnly::LoadFile(const char* filename)
{
	ifstream ifile(filename, ios::in | ios::binary);

	if(ifile.fail() || ifile.eof() || !ifile.good())
		return false;

	ifile.read((char*)&m_memoryData[0], 0x8000);
	ifile.close();

	return true;
}
