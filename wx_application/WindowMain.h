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
#ifndef WindowMain_H
#define WindowMain_H

#include "wx/glcanvas.h"
#include "wx/wx.h"

namespace Emunisce {

class Application;

class WindowMain : public wxGLCanvas {
	// Emunsice stuff

public:
	void SetApplication(Application* application);

private:
	Application* m_application;

	// wx stuff

private:
	wxGLContext* m_context;

public:
	WindowMain(wxFrame* parent, int* args);
	virtual ~WindowMain();

	int getWidth();
	int getHeight();

	// events
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnResize(wxSizeEvent& event);
	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& event);
	void OnIdle(wxIdleEvent& event);

	DECLARE_EVENT_TABLE()
};

}  // namespace Emunisce

#endif  // WindowMain_H
