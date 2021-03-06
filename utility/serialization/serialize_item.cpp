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
#include "serialize_item.h"
using namespace emunisce;

#include "archive.h"

namespace emunisce {

void SerializeBuffer(Archive& archive, unsigned char* buffer, unsigned int bytes) {
	archive.SerializeBuffer(buffer, bytes);
}

void SerializeItem(Archive& archive, u8& data) {
	archive& data;
}

void SerializeItem(Archive& archive, u16& data) {
	archive& data;
}

void SerializeItem(Archive& archive, u32& data) {
	archive& data;
}

void SerializeItem(Archive& archive, u64& data) {
	archive& data;
}

void SerializeItem(Archive& archive, s8& data) {
	archive& data;
}

void SerializeItem(Archive& archive, s16& data) {
	archive& data;
}

void SerializeItem(Archive& archive, s32& data) {
	archive& data;
}

void SerializeItem(Archive& archive, s64& data) {
	archive& data;
}

void SerializeItem(Archive& archive, bool& data) {
	archive& data;
}

void SerializeItem(Archive& archive, float& data) {
	archive& data;
}

void SerializeItem(Archive& archive, double& data) {
	archive& data;
}

}  // namespace emunisce
