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

#include "ConsoleWindow.h"
using namespace Emunisce;

#include "Application.h"

class WtfTextCtrl : public wxTextCtrl
{
public:

    WtfTextCtrl(ConsoleWindow* console, wxWindow *parent, wxWindowID id, const wxString &value = wxEmptyString, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0L, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxTextCtrlNameStr) :
        wxTextCtrl(parent, id, value, pos, size, style, validator, name)
    {
        Connect(wxEVT_KEY_DOWN, wxKeyEventHandler(WtfTextCtrl::OnKeyDown));
        Connect(wxEVT_KEY_UP, wxKeyEventHandler(WtfTextCtrl::OnKeyUp));
        Connect(wxEVT_COMMAND_TEXT_UPDATED, wxTextEventHandler(WtfTextCtrl::OnText));
        Connect(wxEVT_COMMAND_TEXT_ENTER, wxTextEventHandler(WtfTextCtrl::OnTextEnter));
        Connect(wxEVT_SET_FOCUS, wxFocusEventHandler(WtfTextCtrl::OnSetFocus));

        m_console = console;
    }

    void OnKeyDown(wxKeyEvent& event)
    {
        m_console->OnKeyDown(event);
    }

    void OnKeyUp(wxKeyEvent& event)
    {
        m_console->OnKeyUp(event);
    }

    void OnText(wxCommandEvent& event)
    {
        m_console->OnText(event);
    }

    void OnTextEnter(wxCommandEvent& event)
    {
        m_console->OnTextEnter(event);
    }

    void OnSetFocus(wxFocusEvent& event)
    {
        m_console->OnSetFocus(event);
    }


private:

    ConsoleWindow* m_console;
};

class ConsoleFrame : public wxFrame
{
public:

    ConsoleFrame(ConsoleWindow* consoleWindow, wxFrame* parent, wxString& title, wxPoint& position, wxSize& size)
        : wxFrame(parent, wxID_ANY, title, position, size)
    {
        m_consoleWindow = consoleWindow;
    }

    void OnClose(wxCloseEvent& event)
    {
        m_consoleWindow->OnFrameClosed(event);
        event.Skip(true);
    }

private:

    ConsoleWindow* m_consoleWindow;
};

ConsoleWindow::ConsoleWindow(Application* application, wxFrame* mainFrame)
{
    Initialize(application, mainFrame);
}

ConsoleWindow::~ConsoleWindow()
{
}

void ConsoleWindow::Initialize(Application* application, wxFrame* mainFrame)
{
    m_application = application;
    m_mainFrame = mainFrame;

    wxString title(wxT("Emunisce Console"));
    wxSize consoleSize(400, 200);
    wxPoint consolePos = mainFrame->GetPosition();
    consolePos.x += mainFrame->GetSize().GetWidth();

    m_frame = new ConsoleFrame(this, mainFrame, title, consolePos, consoleSize);

    m_output =  new WtfTextCtrl(this, m_frame, wxID_ANY, wxT(""), wxPoint(0,0), wxSize(400,180),
        wxTE_READONLY | wxTE_MULTILINE | wxTE_RICH | wxTE_BESTWRAP | wxTE_AUTO_SCROLL);
    m_output->SetBackgroundColour(*wxBLACK);
    m_output->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK));

    m_input =  new WtfTextCtrl(this, m_frame, wxID_ANY, wxT(""), wxPoint(0,180), wxSize(400,20),
        wxTE_PROCESS_ENTER | wxTE_PROCESS_TAB);
    m_input->SetBackgroundColour(*wxBLACK);
    m_input->SetDefaultStyle(wxTextAttr(*wxLIGHT_GREY, *wxBLACK));

    m_frame->Show();
}

void ConsoleWindow::GiveFocus()
{
    if(m_frame == nullptr)
        Initialize(m_application, m_mainFrame);

    m_frame->Show();
    m_frame->Raise();

    m_input->SetFocus();
}


void ConsoleWindow::OnKeyDown(wxKeyEvent& event)
{
    int key = event.GetKeyCode();

    if(key == (int)'`' || key == WXK_ESCAPE)
    {
        m_application->ShowGameWindow();
        event.Skip(false);
    }

    else
    {
        event.Skip(true);
    }
}

void ConsoleWindow::OnKeyUp(wxKeyEvent& event)
{
    event.Skip(true);
}

void ConsoleWindow::OnText(wxCommandEvent& event)
{
    event.Skip(true);
}

void ConsoleWindow::OnTextEnter(wxCommandEvent& event)
{
    *m_output << m_input->GetValue() << wxT("\n");
    m_input->SetValue(wxT(""));
    event.Skip(true);
}

void ConsoleWindow::OnSetFocus(wxFocusEvent& event)
{
    if(event.GetWindow() != m_input)
    {
        m_input->SetFocus();
    }

    event.Skip(true);
}

void ConsoleWindow::OnFrameClosed(wxCloseEvent& event)
{
    delete m_output;
    m_output = nullptr;

    delete m_input;
    m_input = nullptr;

    delete m_frame;
    m_frame = nullptr;
}