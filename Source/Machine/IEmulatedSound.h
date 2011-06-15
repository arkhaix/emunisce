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
#ifndef IEMULATEDSOUND_H
#define IEMULATEDSOUND_H


static const unsigned int SamplesPerSecond = 44100;

//static const unsigned int BytesPerSample = 1;
//typedef u8 SampleType;
//#define SilentSample ((SampleType)0x80)
//#define MaxSample ((SampleType)0xff)

static const unsigned int BytesPerSample = 2;
typedef s16 SampleType;
#define SilentSample ((SampleType)0x0000)
#define MaxSample ((SampleType)0x7fff)

struct AudioBuffer
{
	static const unsigned int BufferSizeSamples = SamplesPerSecond / 20;	///<Constant is frames (full buffers) per second
	static const unsigned int BufferSizeBytes = BufferSizeSamples * BytesPerSample;

	SampleType Samples[2][BufferSizeSamples];	///<2 channels
};

namespace SquareSynthesisMethod
{
	typedef int Type;

	enum
	{
		Naive = 0,
		LinearInterpolation,

		NumSquareSynthesisMethods
	};
}


class IEmulatedSound
{
public:

	virtual AudioBuffer GetStableAudioBuffer() = 0;
	virtual int GetAudioBufferCount() = 0;

	virtual void SetSquareSynthesisMethod(SquareSynthesisMethod::Type method) = 0;
};

#endif
