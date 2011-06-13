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


WindowSize Window::GetSize()
{
	return m_private->GetSize();
}

void Window::SetSize(WindowSize size)
{
	m_private->SetSize(size);
}


WindowPosition Window::GetPosition()
{
	return m_private->GetPosition();
}

void Window::SetPosition(WindowPosition position)
{
	m_private->SetPosition(position);
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


void Window::PumpMessages()
{
	m_private->PumpMessages();
}

void Window::RequestExit()
{
	m_private->RequestExit();
}
