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
#include "MachineSerialization.h"

#include "MachineIncludes.h"
#include "Serialization/SerializationIncludes.h"

namespace Emunisce {

void SerializeItem(Archive& archive, AudioBuffer& data) {
	SerializeItem(archive, data.NumSamples);
	for (unsigned int i = 0; i < data.NumSamples; i++) {
		SerializeItem(archive, data.Samples[0][i]);
		SerializeItem(archive, data.Samples[1][i]);
	}
}

void SerializeItem(Archive& archive, ScreenBuffer& data) {
	int width = data.GetWidth();
	int height = data.GetHeight();
	DisplayPixel* pixels = data.GetPixels();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			SerializeItem(archive, pixels[y * width + x]);
		}
	}
}

}  // namespace Emunisce
