#include "Window.h"

#include "Window_Internal.h"


Window::Window()
{
	m_private = new Window_Private();
}

Window::~Window()
{
	delete m_private;
}


void Window::Create(int width, int height, const char* title, const char* className)
{
	m_private->Create(width, height, title, className);
}

void Window::Destroy()
{
	m_private->Destroy();
}


void Window::Show()
{
	m_private->Show();
}

void Window::Hide()
{
	m_private->Hide();
}


void* Window::GetHandle()
{
	return m_private->GetHandle();
}


void Window::SubscribeListener(IWindowMessageListener* listener)
{
	m_private->SubscribeListener(listener);
}

void Window::UnsubscribeListener(IWindowMessageListener* listener)
{
	m_private->UnsubscribeListener(listener);
}

