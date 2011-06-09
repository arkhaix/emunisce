#include "windows.h"

#include "phoenix.h"
#include "consoledebugger.h"
#include "gdiPlusRenderer.h"

Phoenix* g_phoenix = NULL;

DWORD WINAPI EmulationThread(LPVOID param)
{
	ConsoleDebugger* debugger = g_phoenix->GetDebugger();
	debugger->Run();

	return 0;
}

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, PSTR, INT iCmdShow)
{
	g_phoenix = new Phoenix();

	HANDLE emulationThreadHandle = CreateThread(NULL, 0, EmulationThread, NULL, 0, NULL);
	
	g_phoenix->GetRenderer()->RunMessagePump();

	WaitForSingleObject(emulationThreadHandle, 1000);

	delete g_phoenix;

	return 0;
}

