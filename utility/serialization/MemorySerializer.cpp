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
using namespace emunisce;

#include <memory.h>
#include <stdlib.h>

// MemorySerializer

MemorySerializer::MemorySerializer() {
	m_usedSize = 0;
	m_reservedSize = 1024;
	m_reserveMultiplier = 1.25f;

	m_buffer = (unsigned char*)malloc(m_reservedSize);
}

MemorySerializer::~MemorySerializer() {
	if (m_buffer != nullptr) {
		free(m_buffer);
	}
}

unsigned char* MemorySerializer::GetBuffer() {
	return m_buffer;
}

unsigned int MemorySerializer::GetBufferSize() {
	return m_usedSize;
}

void MemorySerializer::TransferBuffer(unsigned char** buffer, unsigned int* size) {
	if (buffer == nullptr || size == nullptr) {
		return;
	}

	*buffer = m_buffer;
	*size = m_usedSize;

	m_buffer = nullptr;
	m_usedSize = 0;
	m_reservedSize = 0;

	// todo: re-initialize buffer?
}

void MemorySerializer::SetBuffer(unsigned char* buffer, unsigned int size) {
	free(m_buffer);
	m_reservedSize = size;
	m_usedSize = 0;
	m_buffer = (unsigned char*)malloc(m_reservedSize);
	memcpy(m_buffer, buffer, size);
}

// Serializer

void MemorySerializer::SetArchive(Archive* archive) {
}

void MemorySerializer::Save(unsigned char* data, unsigned int bytes) {
	if (m_usedSize + bytes >= m_reservedSize) {
		float fNewSize = (float)(m_reservedSize + bytes) * m_reserveMultiplier;
		unsigned int newSize = (unsigned int)fNewSize;

		unsigned char* newBuffer = (unsigned char*)malloc(newSize);

		memcpy((void*)newBuffer, m_buffer, m_usedSize);

		if (m_buffer != nullptr) {
			free(m_buffer);
		}

		m_buffer = newBuffer;
		m_reservedSize = newSize;
	}

	memcpy((void*)(m_buffer + m_usedSize), (void*)data, bytes);
	m_usedSize += bytes;
}

void MemorySerializer::Restore(unsigned char* buffer, unsigned int bytes) {
	if (m_buffer == nullptr || buffer == nullptr) {
		return;
	}

	memcpy((void*)buffer, (void*)(m_buffer + m_usedSize), bytes);
	m_usedSize += bytes;
}

void MemorySerializer::Close() {
	// Nothing to do here.  The destructor cleans up the memory when necessary.
}
