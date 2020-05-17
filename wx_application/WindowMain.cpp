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
#include "WindowMain.h"
using namespace Emunisce;

#include <thread>

// Emunisce stuff

#include "Application.h"
#include "PlatformIncludes.h"

void WindowMain::SetApplication(Application* application) {
	m_application = application;
}

// wx stuff

// include OpenGL
#ifdef __WXMAC__
#include "OpenGL/gl.h"
#include "OpenGL/glu.h"
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

BEGIN_EVENT_TABLE(WindowMain, wxGLCanvas)
EVT_KEY_DOWN(WindowMain::OnKeyDown)
EVT_KEY_UP(WindowMain::OnKeyUp)
EVT_SIZE(WindowMain::OnResize)
EVT_PAINT(WindowMain::OnPaint)
EVT_ERASE_BACKGROUND(WindowMain::OnEraseBackground)
EVT_IDLE(WindowMain::OnIdle)
END_EVENT_TABLE()

void WindowMain::OnKeyDown(wxKeyEvent& event) {
	if (m_application != nullptr) {
		if (event.GetKeyCode() == (int)'`')
			m_application->ShowConsoleWindow();
		else
			m_application->KeyDown(event.GetKeyCode());
	}
}

void WindowMain::OnKeyUp(wxKeyEvent& event) {
	if (m_application != nullptr)
		m_application->KeyUp(event.GetKeyCode());
}

void WindowMain::OnResize(wxSizeEvent& event) {
	//	wxGLCanvas::OnSize(evt);

	Refresh();

	if (m_application != nullptr)
		m_application->Resize(event.GetSize().GetWidth(), event.GetSize().GetHeight());
}

void WindowMain::OnPaint(wxPaintEvent& event) {
	if (!IsShown())
		return;

	wxGLCanvas::SetCurrent(*m_context);
	wxPaintDC(this);  // only to be used in paint events. use wxClientDC to paint outside the paint event

	if (m_application != nullptr)
		m_application->Draw();

	glFlush();
	SwapBuffers();
}

void WindowMain::OnEraseBackground(wxEraseEvent& event) {
	// Do nothing.  Prevents flicker on Windows.
}

void WindowMain::OnIdle(wxIdleEvent& event) {
	Refresh();
	std::this_thread::yield();
}

WindowMain::WindowMain(wxFrame* parent, int* args)
	: wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE) {
	m_application = nullptr;

	m_context = new wxGLContext(this);

	// To avoid flashing on MSW
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

WindowMain::~WindowMain() {
	delete m_context;
}

int WindowMain::getWidth() {
	return GetSize().x;
}

int WindowMain::getHeight() {
	return GetSize().y;
}
