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
#include "KingsDream.h"
using namespace Emunisce;

#include "MachineIncludes.h"

#include <math.h>
#include <stdlib.h>
#include <time.h>


KingsDream::KingsDream()
{
	srand((unsigned int)time(nullptr));

	m_screenBuffer = nullptr;
	for (auto& frame : m_frames) {
		frame = nullptr;
	}

	ResizeScreenBuffers(320, 240);

	//"The King's Dream" initial coefficients
	m_x = 0.1f; m_y = 0.1f;
	m_a = -1.6f;
	m_b = 2.879879f;
	m_c = 0.765145f;
	m_d = 0.744728f;

	//Let the generator settle
	for (int i = 0; i < 10000; i++) {
		SilentDream();
	}

	//Clear the screen buffer
	m_screenBuffer->Clear(DisplayPixelFromRGBA((u8)0, (u8)0, (u8)0));

	m_numBlendFrames = 5;
	m_currentFrame = 0;

	m_incrementPerFrame = 0.001f;

	m_brightness = 40;

	m_pointsPerFrame = 10000;
	m_pointsThisFrame = 0;

	m_framesPerColor = 150;

	m_pointsPerColor = m_pointsPerFrame * m_framesPerColor;
	m_pointsThisColor = 0;

	//Skip ranges
	m_skipRanges.push_back(std::make_pair(-4.05f, -4.00f));	///<Jarring blank section
	m_skipRanges.push_back(std::make_pair(-3.73f, -3.67f));	///<Jarring blank section
	m_skipRanges.push_back(std::make_pair(-3.16f, -3.05f));	///<Blank section
	m_skipRanges.push_back(std::make_pair(-2.64f, -2.52f));	///<Blank section
	m_skipRanges.push_back(std::make_pair(-1.88f, -1.72f));	///<Blinky section
	m_skipRanges.push_back(std::make_pair(-0.72f, 0.f));	///<Bad place.  This range murders the cpu.  Probably throws bad values into sin().
	m_skipRanges.push_back(std::make_pair(0.f, 1.72f));	///<Boring section
	//m_skipRanges.push_back( std::make_pair(0.7f, 1.28f) );	///<Blank section	///<Above "boring section" encompasses this
	m_skipRanges.push_back(std::make_pair(2.65f, 3.15f));	///<Blank section
}


void KingsDream::UpdateAnimation()
{
	Dream();

	m_pointsThisFrame++;
	if (m_pointsThisFrame >= m_pointsPerFrame)
	{
		m_pointsThisFrame -= m_pointsPerFrame;

		IncrementGenerator();

		BlendBuffers();

		m_currentFrame++;
		if (m_currentFrame >= m_numBlendFrames) {
			m_currentFrame = 0;
		}

		m_frames[m_currentFrame]->Clear(DisplayPixelFromRGBA((u8)0, (u8)0, (u8)0));
	}
}

ScreenBuffer* KingsDream::GetFrame()
{
	return m_screenBuffer;
}



// Properties

void KingsDream::SetScreenResolution(unsigned int width, unsigned int height)
{
	ResizeScreenBuffers(width, height);
}


void KingsDream::SetBrightness(unsigned int brightness)
{
	m_brightness = brightness;
}

unsigned int KingsDream::GetBrightness()
{
	return m_brightness;
}


void KingsDream::SetFramesPerColor(unsigned int numFrames)
{
	m_framesPerColor = numFrames;
	m_pointsPerColor = m_pointsPerFrame * m_framesPerColor;
}

unsigned int KingsDream::GetFramesPerColor()
{
	return m_framesPerColor;
}


void KingsDream::SetPointsPerFrame(unsigned int numPoints)
{
	m_pointsPerFrame = numPoints;
	m_pointsPerColor = m_pointsPerFrame * m_framesPerColor;
}

unsigned int KingsDream::GetPointsPerFrame()
{
	return m_pointsPerFrame;
}


void KingsDream::SetBlendFrames(unsigned int numFrames)
{
	m_numBlendFrames = numFrames;
}

unsigned int KingsDream::GetBlendFrames()
{
	return m_numBlendFrames;
}


void KingsDream::ResizeScreenBuffers(unsigned int width, unsigned int height)
{
	if (m_screenBuffer != nullptr && m_screenBuffer->GetWidth() == (int)width && m_screenBuffer->GetHeight() == (int)height) {
		return;
	}

	delete m_screenBuffer;
	for (auto & m_frame : m_frames) {
		delete m_frame;
	}

	m_screenBuffer = new DynamicScreenBuffer(width, height);
	for (auto & m_frame : m_frames) {
		m_frame = new DynamicScreenBuffer(width, height);
	}
}


void KingsDream::IncrementGenerator()
{
	m_a += m_incrementPerFrame;

	//Reset after a certain point
	if (m_a > 4.85f) {	///<Arbitrary reset point.
		m_a = -4.45f;	///<Arbitrary start position.
	}

	//Skip bad ranges
	for (auto & m_skipRange : m_skipRanges)
	{
		if (m_a > m_skipRange.first && m_a < m_skipRange.second) {
			m_a += m_skipRange.second - m_skipRange.first;
		}
	}
}

void KingsDream::BlendBuffers()
{
	DisplayPixel* screenPixels = m_screenBuffer->GetPixels();

	DisplayPixel* framePixels[m_maxNumBlendFrames] = { nullptr };
	for (unsigned int i = 0; i < m_numBlendFrames; i++) {
		framePixels[i] = m_frames[i]->GetPixels();
	}

	int numPixels = m_screenBuffer->GetWidth() * m_screenBuffer->GetHeight();

	u8 r, g, b, a;
	int nr, ng, nb;	///<int to handle overflow

	for (int pixelIndex = 0; pixelIndex < numPixels; pixelIndex++)
	{
		nr = 0;
		ng = 0;
		nb = 0;

		for (unsigned int frameIndex = 0; frameIndex < m_numBlendFrames; frameIndex++)
		{
			DisplayPixelToRGBA(framePixels[frameIndex][pixelIndex], r, g, b, a);

			nr += r;
			ng += g;
			nb += b;
		}

		nr /= m_numBlendFrames;
		ng /= m_numBlendFrames;
		nb /= m_numBlendFrames;

		screenPixels[pixelIndex] = DisplayPixelFromRGBA((u8)nr, (u8)ng, (u8)nb);
	}
}


void KingsDream::SilentDream()
{
	float newX = sin(m_y*m_b) + m_c * sin(m_x*m_b);
	float newY = sin(m_x*m_a) + m_d * sin(m_y*m_a);

	m_x = newX;
	m_y = newY;
}

void KingsDream::Dream()
{
	SilentDream();

	int width = m_screenBuffer->GetWidth();
	int height = m_screenBuffer->GetHeight();

	int xPos = (int)(((m_x + 2.f) / 4.f) * width);
	int yPos = (int)(((m_y + 2.f) / 4.f) * height);
	int index = yPos * width + xPos;

	u8 incZero = 0;
	u8 incOne = (u8)m_brightness;

	DisplayPixel* pixels = m_frames[m_currentFrame]->GetPixels();

	static DisplayPixel randomIncrement = DisplayPixelFromRGBA(incOne, incZero, incZero);
	static const DisplayPixel mask = DisplayPixelFromRGBA(incZero, incZero, incZero, (u8)255);

	static DisplayPixel oldIncrement = randomIncrement;
	DisplayPixel blendIncrement = incZero;


	u8 oldR, oldG, oldB, oldA;
	DisplayPixelToRGBA(oldIncrement, oldR, oldG, oldB, oldA);

	u8 newR, newG, newB, newA;
	DisplayPixelToRGBA(randomIncrement, newR, newG, newB, newA);

	float blend = (float)m_pointsThisColor / (float)m_pointsPerColor;
	u8 br = (u8)((newR * blend) + (oldR * (1.f - blend)));
	u8 bg = (u8)((newG * blend) + (oldG * (1.f - blend)));
	u8 bb = (u8)((newB * blend) + (oldB * (1.f - blend)));

	blendIncrement = DisplayPixelFromRGBA(br, bg, bb);


	pixels[index] += blendIncrement;
	pixels[index] |= mask;

	m_pointsThisColor++;
	if (m_pointsThisColor >= m_pointsPerColor)
	{
		m_pointsThisColor = 0;
		oldIncrement = randomIncrement;

		u8 rr = 0;
		u8 rg = 0;
		u8 rb = 0;
		while (rr + rg + rb < incOne)	///<Make sure our random color is at least as bright as the default increment
		{
			rr = rand() % (incOne + 1);
			rg = rand() % (incOne + 1);
			rb = rand() % (incOne + 1);
		}
		randomIncrement = DisplayPixelFromRGBA(rr, rg, rb);
	}
}
