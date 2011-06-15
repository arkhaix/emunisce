#ifndef HQ4X_H
#define HQ4X_H

class ScreenBuffer;

class HqNx
{
public:

	static ScreenBuffer* Hq2x(ScreenBuffer* originalScreen);
	static ScreenBuffer* Hq3x(ScreenBuffer* originalScreen);
	static ScreenBuffer* Hq4x(ScreenBuffer* originalScreen);

	static void Release(ScreenBuffer* buffer);
};

#endif
