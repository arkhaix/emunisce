#ifndef IEMULATEDDISPLAY_H
#define IEMULATEDDISPLAY_H

#include "MachineTypes.h"


namespace DisplayFilter
{
	typedef int Type;

	enum
	{
		None = 0,

		Hq2x,
		Hq3x,
		Hq4x,

		NumDisplayFilters
	};
}


struct ScreenResolution
{
	int width;
	int height;
};


class IEmulatedDisplay
{
public:

	virtual ScreenResolution GetScreenResolution() = 0;	///<Returns the native resolution of the screen.  Should be static per machine.
	virtual ScreenBuffer* GetStableScreenBuffer() = 0;	///<Returns the most recent screen buffer.  Note that the resolution may be different from GetScreenResolution, especially if filters are applied.
	virtual int GetScreenBufferCount() = 0;	///<Returns the id of the current screen buffer.  Not guaranteed to be unique or sequential, so use != when polling for changes.

	virtual void SetFilter(DisplayFilter::Type filter) = 0;
};

#endif
