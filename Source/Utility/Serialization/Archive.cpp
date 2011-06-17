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
#include "Archive.h"
using namespace Emunisce;

#include "ISerializer.h"


Archive::Archive(ISerializer* serializer, ArchiveMode::Type archiveMode)
{
	m_serializer = serializer;
	m_archiveMode = archiveMode;
}


Archive& Archive::operator&(u8& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}

Archive& Archive::operator&(u16& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}

Archive& Archive::operator&(u32& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}

Archive& Archive::operator&(u64& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}


Archive& Archive::operator&(s8& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}

Archive& Archive::operator&(s16& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}

Archive& Archive::operator&(s32& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}

Archive& Archive::operator&(s64& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}


Archive& Archive::operator&(float& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}

Archive& Archive::operator&(double& data)
{
	if(m_archiveMode == ArchiveMode::Saving)
		m_serializer->Save((unsigned char*)&data, sizeof(data));
	else
		m_serializer->Restore((unsigned char*)&data, sizeof(data));

	return *this;
}

