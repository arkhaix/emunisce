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
#define GuiButtons_ToString 1

#include "Gui.h"
using namespace emunisce;

#include <Memory.h>

#include "KingsDream.h"
#include "hqnx/HqNx.h"

// Gui

Gui::Gui() {
	m_screenBufferCopy = nullptr;
	m_filteredScreenBuffer = nullptr;
	m_filteredScreenBufferId = -1;
	m_displayFilter = DisplayFilter::NoFilter;

	m_guiFeature = new GuiFeature();
	m_featureExecution = m_guiFeature;
	m_featureDisplay = m_guiFeature;
	m_featureInput = m_guiFeature;
}

Gui::~Gui() {
	m_featureExecution = nullptr;
	m_featureDisplay = nullptr;
	m_featureInput = nullptr;
	delete m_guiFeature;

	if (m_screenBufferCopy != nullptr) {
		delete m_screenBufferCopy;
	}
}

ScreenBuffer* Gui::GetStableScreenBuffer() {
	if (MachineFeature::GetScreenBufferCount() == m_filteredScreenBufferId &&
		m_displayFilter == m_screenBufferCopyFilter) {
		return m_filteredScreenBuffer;
	}

	ScreenBuffer* screenBuffer = MachineFeature::GetStableScreenBuffer();

	bool needToDeleteFilteredBuffer = false;
	if (m_filteredScreenBuffer != nullptr && m_filteredScreenBuffer != m_screenBufferCopy) {
		needToDeleteFilteredBuffer = true;
	}

	if (m_screenBufferCopy != nullptr) {
		delete m_screenBufferCopy;
	}

	int width = screenBuffer->GetWidth();
	int height = screenBuffer->GetHeight();

	m_screenBufferCopy = new DynamicScreenBuffer(width, height);
	memcpy((void*)m_screenBufferCopy->GetPixels(), (void*)screenBuffer->GetPixels(),
		   width * height * sizeof(DisplayPixel));

	if (needToDeleteFilteredBuffer == true) {
		delete m_filteredScreenBuffer;
	}

	m_filteredScreenBuffer = m_screenBufferCopy;
	m_filteredScreenBufferId = MachineFeature::GetScreenBufferCount();
	m_screenBufferCopyFilter = m_displayFilter;

	if (m_displayFilter == DisplayFilter::Hq2x) {
		m_filteredScreenBuffer = HqNx::Hq2x(m_screenBufferCopy);
	}
	else if (m_displayFilter == DisplayFilter::Hq3x) {
		m_filteredScreenBuffer = HqNx::Hq3x(m_screenBufferCopy);
	}
	else if (m_displayFilter == DisplayFilter::Hq4x) {
		m_filteredScreenBuffer = HqNx::Hq4x(m_screenBufferCopy);
	}

	return m_filteredScreenBuffer;
}

void Gui::EnableBackgroundAnimation() {
	if (m_guiFeature != nullptr) {
		m_guiFeature->EnableBackgroundAnimation();
	}
}

void Gui::DisableBackgroundAnimation() {
	if (m_guiFeature != nullptr) {
		m_guiFeature->DisableBackgroundAnimation();
	}
}

void Gui::SetDisplayFilter(DisplayFilter::Type filter) {
	m_displayFilter = filter;
}

// GuiFeature

Gui::GuiFeature::GuiFeature() {
	m_backgroundAnimation = new KingsDream();
	m_backgroundEnabled = true;

	m_backgroundAnimation->SetScreenResolution(m_screenBuffer.GetWidth(), m_screenBuffer.GetHeight());

	m_ticksThisFrame = 0;
	m_ticksPerFrame = m_backgroundAnimation->GetPointsPerFrame() /
					  2;  ///< Only update the background at 30fps.  This is because it uses a bit of cpu.  Don't forget
						  ///< to update the brightness (or points per frame) if you change this.
	m_frameCount = 0;
}

void Gui::GuiFeature::EnableBackgroundAnimation() {
	m_backgroundEnabled = true;
}

void Gui::GuiFeature::DisableBackgroundAnimation() {
	m_backgroundEnabled = false;
}

// IExecutableFeature

unsigned int Gui::GuiFeature::GetFrameCount() {
	return m_frameCount;
}

unsigned int Gui::GuiFeature::GetTickCount() {
	return m_ticksThisFrame;
}

unsigned int Gui::GuiFeature::GetTicksPerSecond() {
	return m_ticksPerFrame * 60;
}

unsigned int Gui::GuiFeature::GetTicksUntilNextFrame() {
	return m_ticksPerFrame - m_ticksThisFrame;
}

void Gui::GuiFeature::Step() {
	m_ticksThisFrame++;
	if (m_ticksThisFrame >= m_ticksPerFrame) {
		m_ticksThisFrame = 0;
		m_frameCount++;
	}

	if (m_backgroundEnabled == true) {
		m_backgroundAnimation->UpdateAnimation();
	}
}

void Gui::GuiFeature::RunToNextFrame() {
	int frameCount = m_frameCount;
	while (m_frameCount == frameCount) {
		Step();
	}
}

// IEmulatedDisplay

ScreenResolution Gui::GuiFeature::GetScreenResolution() {
	ScreenResolution result;
	result.width = m_screenBuffer.GetWidth();
	result.height = m_screenBuffer.GetHeight();
	return result;
}

ScreenBuffer* Gui::GuiFeature::GetStableScreenBuffer() {
	ScreenBuffer* result = &m_screenBuffer;
	if (m_backgroundEnabled == true) {
		result = m_backgroundAnimation->GetFrame();
	}

	return result;
}

int Gui::GuiFeature::GetScreenBufferCount() {
	return m_frameCount;
}

// IEmulatedInput

unsigned int Gui::GuiFeature::NumButtons() {
	return GuiButtons::NumGuiButtons;
}

const char* Gui::GuiFeature::GetButtonName(unsigned int index) {
	if (index < GuiButtons::NumGuiButtons) {
		return GuiButtons::ToString[index];
	}

	return nullptr;
}

void Gui::GuiFeature::ButtonDown(unsigned int index) {
	if (index >= GuiButtons::NumGuiButtons) {
		return;
	}

	// todo
}

void Gui::GuiFeature::ButtonUp(unsigned int index) {
	if (index >= GuiButtons::NumGuiButtons) {
		return;
	}

	// todo
}

bool Gui::GuiFeature::IsButtonDown(unsigned int /*index*/) {
	// todo
	return false;
}
