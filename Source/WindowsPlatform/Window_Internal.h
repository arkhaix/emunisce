#ifndef WINDOW_INTERNAL_H
#define WINDOW_INTERNAL_H

#include "windows.h"

#include <list>
#include <set>
using namespace std;

#include "Window.h"
#include "Mutex.h"

class Window_Private
{
public:

	Window_Private();
	~Window_Private();

	void Create(int width, int height, const char* title, const char* className);
	void Destroy();

	void* GetHandle();

	void SubscribeListener(IWindowMessageListener* listener);
	void UnsubscribeListener(IWindowMessageListener* listener);

	void Show();
	void Hide();

	WindowSize GetSize();
	void SetSize(WindowSize size);

	WindowPosition GetPosition();
	void SetPosition(WindowPosition position);

private:

	static LRESULT CALLBACK StaticWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static set<Window_Private*> m_validInstances;
	static Mutex m_validInstancesLock;

	LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	bool m_needsDestroy;

	HWND m_handle;

	WindowSize m_size;
	WindowPosition m_position;

	list<IWindowMessageListener*> m_listeners;
};

#endif
