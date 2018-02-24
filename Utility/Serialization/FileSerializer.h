/*
Copyright (C) 2011 by Andrew Gray
arkhaix@emunisce.com

This file is part of Emunisce.

Emunisce is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

Emunisce is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Emunisce.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FILESERIALIZER_H
#define FILESERIALIZER_H

#include "ISerializer.h"

#include <fstream>


namespace Emunisce
{

class FileSerializer : public ISerializer
{
public:

	// FileSerializer

	FileSerializer();
	~FileSerializer();

	virtual void SetFile(const char* filename);
	virtual void CloseFile();


	// ISerializer

	virtual void SetArchive(Archive* archive);

	virtual void Save(unsigned char* data, unsigned int bytes);
	virtual void Restore(unsigned char* buffer, unsigned int bytes);

	virtual void Close();


protected:

	void OpenStream();

	std::fstream* m_fileStream;

	int m_archiveMode;
	char m_filename[1024];
};

}	//namespace Emunisce

#endif
