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

// Emunsice stuff

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
EVT_MOTION(WindowMain::mouseMoved)
EVT_LEFT_DOWN(WindowMain::mouseDown)
EVT_LEFT_UP(WindowMain::mouseReleased)
EVT_RIGHT_DOWN(WindowMain::rightClick)
EVT_LEAVE_WINDOW(WindowMain::mouseLeftWindow)
EVT_SIZE(WindowMain::resized)
EVT_KEY_DOWN(WindowMain::keyPressed)
EVT_KEY_UP(WindowMain::keyReleased)
EVT_MOUSEWHEEL(WindowMain::mouseWheelMoved)
EVT_PAINT(WindowMain::render)
EVT_IDLE(WindowMain::OnIdle)
END_EVENT_TABLE()


// some useful events to use
void WindowMain::mouseMoved(wxMouseEvent& event) {}
void WindowMain::mouseDown(wxMouseEvent& event) {}
void WindowMain::mouseWheelMoved(wxMouseEvent& event) {}
void WindowMain::mouseReleased(wxMouseEvent& event) {}
void WindowMain::rightClick(wxMouseEvent& event) {}
void WindowMain::mouseLeftWindow(wxMouseEvent& event) {}

void WindowMain::keyPressed(wxKeyEvent& event)
{
    //todo
}

void WindowMain::keyReleased(wxKeyEvent& event)
{
    //todo
}

void WindowMain::OnIdle(wxIdleEvent &event)
{
    Refresh();
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

void WindowMain::resized(wxSizeEvent& evt)
{
//	wxGLCanvas::OnSize(evt);

    Refresh();

    if(m_application != NULL)
        m_application->Resize(evt.GetSize().GetWidth(), evt.GetSize().GetHeight());
}

int WindowMain::getWidth()
{
    return GetSize().x;
}

int WindowMain::getHeight()
{
    return GetSize().y;
}


void WindowMain::render( wxPaintEvent& evt )
{
    if(!IsShown()) return;

    wxGLCanvas::SetCurrent(*m_context);
    wxPaintDC(this); // only to be used in paint events. use wxClientDC to paint outside the paint event

	if(m_application != NULL)
		m_application->Draw();

	glFlush();
	SwapBuffers();
}

