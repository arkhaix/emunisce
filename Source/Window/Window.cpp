#include "Window.h"

#include "windows.h"


class Window_Private
{
public:


};


void Window::Create(int width, int height, const char* title, const char* className)
{
}

void Window::Destroy()
{
}


void Window::Show()
{
}

void Window::Hide()
{
}


void* Window::GetHandle()
{
	return NULL;
}


void Window::SubscribeListener(IWindowMessageListener* listener)
{
}

void Window::UnsubscribeListener(IWindowMessageListener* listener)
{
}

