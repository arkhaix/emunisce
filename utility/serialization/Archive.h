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
#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "PlatformTypes.h"

namespace emunisce {

class ISerializer;

namespace ArchiveMode {
typedef int Type;

enum {
	Saving = 0,
	Loading,

	NumArchiveModes
};
}  // namespace ArchiveMode

class Archive {
public:
	Archive(ISerializer* serializer, ArchiveMode::Type archiveMode);
	void Close();

	ISerializer* GetSerializer();
	ArchiveMode::Type GetArchiveMode();

	void SerializeBuffer(unsigned char* buffer, unsigned int bytes);

	Archive& operator&(u8& data);
	Archive& operator&(u16& data);
	Archive& operator&(u32& data);
	Archive& operator&(u64& data);

	Archive& operator&(s8& data);
	Archive& operator&(s16& data);
	Archive& operator&(s32& data);
	Archive& operator&(s64& data);

	Archive& operator&(bool& data);
	Archive& operator&(float& data);
	Archive& operator&(double& data);

protected:
	ISerializer* m_serializer;
	ArchiveMode::Type m_archiveMode;
};

}  // namespace emunisce

#endif
