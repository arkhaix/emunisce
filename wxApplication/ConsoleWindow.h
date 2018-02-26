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
#ifndef CONSOLEWINDOW_H
#define CONSOLEWINDOW_H

#include "wx/wx.h"
#include "wx/textctrl.h"

#include <string>


namespace Emunisce
{

class Application;

class ConsoleWindow
{
public:
	ConsoleWindow(Application* application, wxFrame* mainFrame);
	virtual ~ConsoleWindow();

    void GiveFocus();
    void Close();

    void ConsolePrint(const char* text);

	// events
	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
    void OnText(wxCommandEvent& event);
    void OnTextEnter(wxCommandEvent& event);
    void OnSetFocus(wxFocusEvent& event);

    void OnFrameClosed(wxCloseEvent& event);

private:

    void Initialize(Application* application, wxFrame* mainFrame);

    Application* m_application;
    wxFrame* m_mainFrame;

    wxFrame* m_frame;
    wxTextCtrl*	m_output;
    wxTextCtrl*	m_input;

    wxBoxSizer* m_inputSizer;
    wxBoxSizer* m_frameSizer;
};

}   //namespace Emunisce

#endif // CONSOLEWINDOW_H

