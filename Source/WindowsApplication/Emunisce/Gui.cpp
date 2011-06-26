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

#include "HqNx/HqNx.h"

#include <memory.h>


// Gui

Gui::Gui()
{
	m_screenBufferCopy = NULL;
    m_filteredScreenBuffer = NULL;
	m_filteredScreenBufferId = -1;
	m_displayFilter = DisplayFilter::None;

	m_guiFeature = new GuiFeature();
	m_featureExecution = m_guiFeature;
	m_featureDisplay = m_guiFeature;
	m_featureInput = m_guiFeature;
}

Gui::~Gui()
{
	m_featureExecution = NULL;
	m_featureDisplay = NULL;
	m_featureInput = NULL;
	delete m_guiFeature;

	if(m_screenBufferCopy != NULL)
        delete m_screenBufferCopy;
}

ScreenBuffer* Gui::GetStableScreenBuffer()
{
    if(MachineFeature::GetScreenBufferCount() == m_filteredScreenBufferId && m_displayFilter == m_screenBufferCopyFilter)
        return m_filteredScreenBuffer;

    ScreenBuffer* screenBuffer = MachineFeature::GetStableScreenBuffer();

    bool needToDeleteFilteredBuffer = false;
    if(m_filteredScreenBuffer != NULL && m_filteredScreenBuffer != m_screenBufferCopy)
        needToDeleteFilteredBuffer = true;

	if(m_screenBufferCopy != NULL)
		delete m_screenBufferCopy;

	int width = screenBuffer->GetWidth();
	int height = screenBuffer->GetHeight();

	m_screenBufferCopy = new DynamicScreenBuffer(width, height);
	memcpy((void*)m_screenBufferCopy->GetPixels(), (void*)screenBuffer->GetPixels(), width * height * sizeof(DisplayPixel));

    if(needToDeleteFilteredBuffer == true)
        delete m_filteredScreenBuffer;

    m_filteredScreenBuffer = m_screenBufferCopy;
    m_filteredScreenBufferId = MachineFeature::GetScreenBufferCount();
	m_screenBufferCopyFilter = m_displayFilter;

    if(m_displayFilter == DisplayFilter::Hq2x)
		m_filteredScreenBuffer = HqNx::Hq2x(m_screenBufferCopy);
	else if(m_displayFilter == DisplayFilter::Hq3x)
		m_filteredScreenBuffer = HqNx::Hq3x(m_screenBufferCopy);
	else if(m_displayFilter == DisplayFilter::Hq4x)
		m_filteredScreenBuffer = HqNx::Hq4x(m_screenBufferCopy);

	return m_filteredScreenBuffer;
}

void Gui::SetFilter(DisplayFilter::Type filter)
{
    m_displayFilter = filter;
}



// GuiFeature

Gui::GuiFeature::GuiFeature()
{
	m_ticksPerFrame = m_screenBuffer.GetWidth() * m_screenBuffer.GetHeight();
	m_ticksUntilNextFrame = m_ticksPerFrame;
	m_frameCount = 0;

	for(int y=0;y<m_screenBuffer.GetHeight();y++)
		for(int x=0;x<m_screenBuffer.GetWidth();x++)
			m_screenBuffer.SetPixel(x, y, DisplayPixelFromRGBA(0.f, 0.f, 0.f));

	m_x = 0.1; m_y = 0.1;               // starting point
	m_a = -0.966918;                  // coefficients for "The King's Dream"
	m_b = 2.879879;
	m_c = 0.765145;
	m_d = 0.744728;

	for(int i=0;i<10000;i++)
		SilentDream();
}


// IExecutableFeature

unsigned int Gui::GuiFeature::GetFrameCount()
{
	return m_frameCount;
}

unsigned int Gui::GuiFeature::GetTicksPerSecond()
{
	return m_ticksPerFrame * 60;
}

unsigned int Gui::GuiFeature::GetTicksUntilNextFrame()
{
	return m_ticksUntilNextFrame;
}

void Gui::GuiFeature::Step()
{
	m_ticksUntilNextFrame--;
	if(m_ticksUntilNextFrame <= 0)
	{
		m_ticksUntilNextFrame += m_ticksPerFrame;
		m_frameCount++;

		for(int i=0;i<1000;i++)	///<Arbitrary constant.  How many pixels are calculated and drawn per frame.  Basically controls how fast it fades in.
			Dream();
	}
}

void Gui::GuiFeature::RunToNextFrame()
{
	int frameCount = m_frameCount;
	while(m_frameCount == frameCount)
		Step();
}


// IEmulatedDisplay

ScreenResolution Gui::GuiFeature::GetScreenResolution()
{
	ScreenResolution result;
	result.width = m_screenBuffer.GetWidth();
	result.height = m_screenBuffer.GetHeight();
	return result;
}

ScreenBuffer* Gui::GuiFeature::GetStableScreenBuffer()
{
	return &m_screenBuffer;
}

int Gui::GuiFeature::GetScreenBufferCount()
{
	return m_frameCount;
}


void Gui::GuiFeature::SetFilter(DisplayFilter::Type filter)
{
}



// IEmulatedInput

unsigned int Gui::GuiFeature::NumButtons()
{
	return GuiButtons::NumGuiButtons;
}

const char* Gui::GuiFeature::GetButtonName(unsigned int index)
{
	if(index < GuiButtons::NumGuiButtons)
		return GuiButtons::ToString[index];

	return NULL;
}


void Gui::GuiFeature::ButtonDown(unsigned int index)
{
	if(index >= GuiButtons::NumGuiButtons)
		return;

	//todo
}

void Gui::GuiFeature::ButtonUp(unsigned int index)
{
	if(index >= GuiButtons::NumGuiButtons)
		return;

	//todo
}

#include <math.h>
void Gui::GuiFeature::SilentDream()
{
	float newX = sin(m_y*m_b) + m_c*sin(m_x*m_b);
	float newY = sin(m_x*m_a) + m_d*sin(m_y*m_a);

	m_x = newX;
	m_y = newY;
}

void Gui::GuiFeature::Dream()
{
	SilentDream();

	static const int width = m_screenBuffer.GetWidth();
	static const int height = m_screenBuffer.GetHeight();

	int xPos = ((m_x+2.f)/4.f) * width;
	int yPos = ((m_y+2.f)/4.f) * height;
	int index = yPos * width + xPos;

	DisplayPixel* pixels = m_screenBuffer.GetPixels();

	static DisplayPixel increment = DisplayPixelFromRGBA((u8)1, (u8)1, (u8)1);
	static DisplayPixel mask = DisplayPixelFromRGBA((u8)0, (u8)0, (u8)0, (u8)255);

	pixels[index] += increment;
	pixels[index] |= mask;
}

