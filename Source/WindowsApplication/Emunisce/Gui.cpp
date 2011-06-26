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

#include <math.h>
#include <memory.h>
#include <stdlib.h>


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
	m_ticksPerFrame = 10000;	///<How many times to update the attractor per frame
	m_ticksUntilNextFrame = m_ticksPerFrame;
	m_frameCount = 0;

	for(int y=0;y<m_screenBuffer.GetHeight();y++)
		for(int x=0;x<m_screenBuffer.GetWidth();x++)
			m_screenBuffer.SetPixel(x, y, DisplayPixelFromRGBA(0.f, 0.f, 0.f));

	//"The King's Dream" initial coefficients
	m_x = 0.1f; m_y = 0.1f;
	m_a = -4.45f;
	m_b = 2.879879f;
	m_c = 0.765145f;
	m_d = 0.744728f;

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
	Dream();

	m_ticksUntilNextFrame--;
	if(m_ticksUntilNextFrame <= 0)
	{
		m_ticksUntilNextFrame += m_ticksPerFrame;
		m_frameCount++;


		//Select the next attractor if necessary

		m_framesThisAttractor++;
		if(m_framesThisAttractor >= 4)	///<Arbitrary constant.  Affects the animation rate and the brightness of the image (larger value = slower animation and brighter image).
		{
			m_a += 0.002;	///<Arbitrary constant.  Partially determines the animation rate.

			if(m_a > -0.72f && m_a < 0.f)	///<Bad place.  This range murders the cpu.  Probably throws bad values into sin().
				m_a += 0.72f;

			if(m_a > 0.85f && m_a < 1.15f)	///<Blank
				m_a += 0.3f;

			if(m_a > 2.65f && m_a < 3.15f)	///<Blank
				m_a += 0.5f;

			if(m_a > 4.85f)	///<Arbitrary reset point.
				m_a = -4.45f;	///<Arbitrary start position.


			//Update the display

			DisplayPixel* screenPixels = m_screenBuffer.GetPixels();

			DisplayPixel* attractorPixels[m_numAttractors];
			for(int i=0;i<m_numAttractors;i++)
				attractorPixels[i] = m_attractorBuffer[i].GetPixels();

			int numPixels = m_screenBuffer.GetWidth() * m_screenBuffer.GetHeight();
			u8 r, g, b, a;
			int nr, ng, nb;
			for(int i=0;i<numPixels;i++)
			{
				nr = 0;
				ng = 0;
				nb = 0;

				for(int j=0;j<m_numAttractors;j++)
				{
					DisplayPixelToRGBA(attractorPixels[j][i], r, g, b, a);
					nr += r;
					ng += g;
					nb += b;
				}

				nr /= m_numAttractors;
				ng /= m_numAttractors;
				nb /= m_numAttractors;

				screenPixels[i] = DisplayPixelFromRGBA((u8)nr, (u8)ng, (u8)nb);
			}


			m_framesThisAttractor = 0;

			m_currentAttractorBuffer++;
			if(m_currentAttractorBuffer >= m_numAttractors)
				m_currentAttractorBuffer = 0;


			//Clear the attractor screen buffer

			DisplayPixel black = DisplayPixelFromRGBA((u8)0, (u8)0, (u8)0);
			numPixels = m_attractorBuffer[ m_currentAttractorBuffer ].GetWidth() * m_attractorBuffer[ m_currentAttractorBuffer ].GetHeight();
			DisplayPixel* pixels = m_attractorBuffer[ m_currentAttractorBuffer ].GetPixels();
			for(int i=0;i<numPixels;i++)
				pixels[i] = black;
		}
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

    static const u8 incZero = 0;
    static const u8 incOne = 5;
    static const DisplayPixel increments[8] =
    {
        DisplayPixelFromRGBA(incZero, incZero, incOne),
        DisplayPixelFromRGBA(incZero, incZero, incOne),
        DisplayPixelFromRGBA(incZero, incOne, incZero),
        DisplayPixelFromRGBA(incZero, incOne, incOne),
        DisplayPixelFromRGBA(incOne, incZero, incZero),
        DisplayPixelFromRGBA(incOne, incZero, incOne),
        DisplayPixelFromRGBA(incOne, incOne, incZero),
        DisplayPixelFromRGBA(incOne, incOne, incOne)
    };
    static int count = 3000000-1;
    static int incIndex = 0;

	DisplayPixel* pixels = m_attractorBuffer[ m_currentAttractorBuffer ].GetPixels();

	static DisplayPixel increment = DisplayPixelFromRGBA((u8)5, (u8)5, (u8)5);		///<Arbitrary constant.  Combines with the number of iterations done to determine the brightness of the image.
	static DisplayPixel mask = DisplayPixelFromRGBA((u8)0, (u8)0, (u8)0, (u8)255);

	static DisplayPixel randomIncrement = increment;

    //pixels[index] += increment;
	//pixels[index] += increments[incIndex];
	pixels[index] += randomIncrement;
	pixels[index] |= mask;

	count++;
	if((count % 3000000) == 0)
	{
	    incIndex = (incIndex+1) & 7;

	    u8 rr, rg, rb;
	    rr = rand() % (incOne+1);
	    rg = rand() % (incOne+1);
	    rb = rand() % (incOne+1);
	    randomIncrement = DisplayPixelFromRGBA(rr, rg, rb);
	}
}

