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

// Emunisce stuff

#include "PlatformIncludes.h"

#include "Application.h"

void WindowMain::SetApplication(Application* application)
{
    m_application = application;
}


// wx stuff

// include OpenGL
#ifdef __WXMAC__
#include "OpenGL/glu.h"
#include "OpenGL/gl.h"
#else
#include <GL/glu.h>
#include <GL/gl.h>
#endif


BEGIN_EVENT_TABLE(WindowMain, wxGLCanvas)
EVT_KEY_DOWN(WindowMain::OnKeyDown)
EVT_KEY_UP(WindowMain::OnKeyUp)
EVT_SIZE(WindowMain::OnResize)
EVT_PAINT(WindowMain::OnPaint)
EVT_ERASE_BACKGROUND(WindowMain::OnEraseBackground)
EVT_IDLE(WindowMain::OnIdle)
END_EVENT_TABLE()


void WindowMain::OnKeyDown(wxKeyEvent& event)
{
    if(m_application != NULL)
        m_application->KeyDown(event.GetKeyCode());
}

void WindowMain::OnKeyUp(wxKeyEvent& event)
{
    if(m_application != NULL)
        m_application->KeyUp(event.GetKeyCode());
}

void WindowMain::OnResize(wxSizeEvent& event)
{
//	wxGLCanvas::OnSize(evt);

    Refresh();

    if(m_application != NULL)
        m_application->Resize(event.GetSize().GetWidth(), event.GetSize().GetHeight());
}

void WindowMain::OnPaint(wxPaintEvent& event)
{
    if(!IsShown()) return;

    wxGLCanvas::SetCurrent(*m_context);
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

	if(m_application != NULL)
		m_application->Draw();

	glFlush();
	SwapBuffers();
}

void WindowMain::OnEraseBackground(wxEraseEvent& event)
{
    //Do nothing.  Prevents flicker on Windows.
}

void WindowMain::OnIdle(wxIdleEvent &event)
{
    Refresh();
	Thread::Sleep(1);
}



WindowMain::WindowMain(wxFrame* parent, int* args) :
    wxGLCanvas(parent, wxID_ANY, args, wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE)
{
    m_application = NULL;


	m_context = new wxGLContext(this);

    // To avoid flashing on MSW
    SetBackgroundStyle(wxBG_STYLE_CUSTOM);
}

WindowMain::~WindowMain()
{
	delete m_context;
}

int WindowMain::getWidth()
{
    return GetSize().x;
}

int WindowMain::getHeight()
{
    return GetSize().y;
}


