/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
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