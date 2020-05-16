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
#include "FileSerializer.h"
using namespace Emunisce;

#include "Archive.h"

#include <stdio.h>
#include <string.h>


FileSerializer::FileSerializer()
{
	m_filename = "";
	m_fileStream = nullptr;

	m_archiveMode = -1;
}

FileSerializer::~FileSerializer()
{
	CloseFile();
}


void FileSerializer::SetFile(const char* filename)
{
	m_filename = filename;
	OpenStream();
}

void FileSerializer::CloseFile()
{
	if (m_fileStream != nullptr)
	{
		m_fileStream->close();
		delete m_fileStream;
		m_fileStream = nullptr;
	}
}



// ISerializer

void FileSerializer::SetArchive(Archive* archive)
{
	if (archive == nullptr) {
		return;
	}

	m_archiveMode = (int)archive->GetArchiveMode();
	OpenStream();
}

void FileSerializer::Save(unsigned char* data, unsigned int bytes)
{
	if (m_fileStream == nullptr || m_fileStream->fail()) {
		return;
	}

	m_fileStream->write((const char*)data, bytes);
}

void FileSerializer::Restore(unsigned char* buffer, unsigned int bytes)
{
	if (m_fileStream == nullptr || m_fileStream->fail()) {
		return;
	}

	m_fileStream->read((char*)buffer, bytes);
}

void FileSerializer::Close()
{
	CloseFile();
}



void FileSerializer::OpenStream()
{
	if (m_fileStream != nullptr) {
		return;
	}

	if (m_filename.length() == 0) {
		return;
	}

	if (m_archiveMode < 0 || m_archiveMode >= ArchiveMode::NumArchiveModes) {
		return;
	}

	if (m_archiveMode == ArchiveMode::Saving)
	{
		m_fileStream = new std::fstream();
		m_fileStream->open(m_filename.c_str(), std::ios::out | std::ios::binary);

		if (m_fileStream->fail())
		{
			delete m_fileStream;
			m_fileStream = nullptr;
		}
	}
	else if (m_archiveMode == ArchiveMode::Loading)
	{
		m_fileStream = new std::fstream();
		m_fileStream->open(m_filename.c_str(), std::ios::in | std::ios::binary);

		if (m_fileStream->fail())
		{
			delete m_fileStream;
			m_fileStream = nullptr;
		}
	}
}
