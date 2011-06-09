#ifndef GDIPLUSRENDERER_H
#define GDIPLUSRENDERER_H

#include "windows.h"

class Phoenix;
class Machine;

class GdiPlusRenderer
{
public:

	void Initialize(Phoenix* phoenix, HWND window = NULL);
	void Shutdown();

	void SetMachine(Machine* machine);

	HWND GetTargetWindow();
	void RunMessagePump();

private:

	static GdiPlusRenderer* m_defaultInstance;
	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	friend class GdiPlusRenderer_Private;
	GdiPlusRenderer_Private* m_private;
};

#endif
