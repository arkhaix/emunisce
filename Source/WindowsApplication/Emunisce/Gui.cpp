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
#include "Gui.h"
using namespace Emunisce;


// Gui

Gui::Gui()
{
	m_guiDisplay = new GuiDisplay();
	m_featureDisplay = m_guiDisplay;
	
	m_guiInput = new GuiInput();
	m_featureInput = m_guiInput;
}

Gui::~Gui()
{
	m_featureDisplay = NULL;
	delete m_guiDisplay;

	m_featureInput = NULL;
	delete m_guiInput;
}



// IEmulatedDisplay

ScreenResolution Gui::GuiDisplay::GetScreenResolution()
{
	ScreenResolution result;
	result.width = 640;
	result.height = 480;
	return result;
}

ScreenBuffer* Gui::GuiDisplay::GetStableScreenBuffer()
{
	return &m_screenBuffer;
}

int Gui::GuiDisplay::GetScreenBufferCount()
{
	int result = m_screenBufferCount;
	m_screenBufferCount++;
	return result;
}


void Gui::GuiDisplay::SetFilter(DisplayFilter::Type filter)
{
	//todo
}



// IEmulatedInput

unsigned int Gui::GuiInput::NumButtons()
{
	return GuiButtons::NumGuiButtons;
}

const char* Gui::GuiInput::GetButtonName(unsigned int index)
{
	if(index < GuiButtons::NumGuiButtons)
		return GuiButtons::ToString[index];

	return NULL;
}


void Gui::GuiInput::ButtonDown(unsigned int index)
{
	if(index >= GuiButtons::NumGuiButtons)
		return;

	//todo
}

void Gui::GuiInput::ButtonUp(unsigned int index)
{
	if(index >= GuiButtons::NumGuiButtons)
		return;

	//todo
}
