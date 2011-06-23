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
#ifndef MEMORYSERIALIZER_H
#define MEMORYSERIALIZER_H

#include "ISerializer.h"

namespace Emunisce
{

class MemorySerializer : public ISerializer
{
public:

	// MemorySerializer

	MemorySerializer();
	virtual ~MemorySerializer();

	virtual unsigned char* GetBuffer();
	virtual unsigned int GetBufferSize();

	virtual void SetBuffer(unsigned char* buffer, unsigned int size);


	// ISerializer

	virtual void SetArchive(Archive* archive);

	virtual void Save(unsigned char* data, unsigned int bytes);
	virtual void Restore(unsigned char* buffer, unsigned int bytes);


protected:

	unsigned char* m_buffer;

	unsigned int m_usedSize;
	unsigned int m_reservedSize;
	float m_reserveMultiplier;

	unsigned int m_restorePosition;
};

}	//namespace Emunisce

#endif