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
#include "MemorySerializer.h"
using namespace Emunisce;

#include <memory.h>
#include <stdlib.h>


// MemorySerializer

MemorySerializer::MemorySerializer()
{
	m_usedSize = 0;
	m_reservedSize = 1024;

	m_buffer = (unsigned char*)malloc(m_reservedSize);
}

MemorySerializer::~MemorySerializer()
{
	if(m_buffer != NULL)
		free(m_buffer);
}


unsigned char* MemorySerializer::GetBuffer()
{
	return m_buffer;
}

unsigned int MemorySerializer::GetBufferSize()
{
	return m_usedSize;
}



// ISerializer

void MemorySerializer::SetArchive(Archive* archive)
{
}

void MemorySerializer::Save(unsigned char* data, unsigned int bytes)
{
	if(m_usedSize + bytes >= m_reservedSize)
	{
		float fNewSize = (float)(m_reservedSize + bytes) * m_reserveMultiplier;
		unsigned int newSize = (unsigned int)fNewSize;

		unsigned char* newBuffer = (unsigned char*)malloc(newSize);

		memcpy_s((void*)newBuffer, newSize, m_buffer, m_usedSize);

		if(m_buffer != NULL)
			free(m_buffer);

		m_buffer = newBuffer;
		m_reservedSize = newSize;
	}

	memcpy_s((void*)(m_buffer+ m_usedSize), m_reservedSize - m_usedSize, (void*)data, bytes);
	m_usedSize += bytes;
}

void MemorySerializer::Restore(unsigned char* buffer, unsigned int bytes)
{
	memcpy_s((void*)buffer, bytes, (void*)(m_buffer + m_usedSize), bytes);
	m_usedSize += bytes;
}

