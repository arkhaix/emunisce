/*
Copyright (C) 2011 by Andrew Gray
arkhaix@arkhaix.com

This file is part of PhoenixGB.

PhoenixGB is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
version 2 as published by the Free Software Foundation.
The full license is available at http://www.gnu.org/licenses/old-licenses/gpl-2.0.txt

PhoenixGB is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with PhoenixGB.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "OpenGLRenderer.h"

#include "windows.h"

#include "gl/gl.h"

#include "../WindowsPlatform/Window.h"

#include "../Machine/Machine.h"
#include "../Display/Display.h"

#include "../Phoenix/Phoenix.h"


class OpenGLRenderer_Private
{
public:

	Phoenix* _Phoenix;

	HWND _WindowHandle;
	HDC _DeviceContext;
	HGLRC _RenderContext;

	OpenGLRenderer_Private()
	{
		_Phoenix = NULL;

		_WindowHandle = NULL;
		_DeviceContext = NULL;
		_RenderContext = NULL;
	}

	void InitializeOpenGL()
	{
		if(_WindowHandle == NULL)
			return;

		_DeviceContext = GetDC(_WindowHandle);

		PIXELFORMATDESCRIPTOR desiredPixelFormat;
		ZeroMemory( &desiredPixelFormat, sizeof( desiredPixelFormat ) );
		desiredPixelFormat.nSize = sizeof( desiredPixelFormat );

		desiredPixelFormat.nVersion = 1;
		desiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
		desiredPixelFormat.cColorBits = 24;
		desiredPixelFormat.cDepthBits = 16;
		desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

		int pixelFormat;
		pixelFormat = ChoosePixelFormat( _DeviceContext, &desiredPixelFormat );
		SetPixelFormat( _DeviceContext, pixelFormat, &desiredPixelFormat );

		_RenderContext = wglCreateContext(_DeviceContext);
	}

	void ShutdownOpenGL()
	{
		wglDeleteContext(_RenderContext);
		ReleaseDC(_WindowHandle, _DeviceContext);
	}
};


void OpenGLRenderer::Initialize(Phoenix* phoenix)
{
	m_private->_Phoenix = phoenix;
	m_private->_WindowHandle = (HWND)phoenix->GetWindow()->GetHandle();

	m_private->InitializeOpenGL();
}

void OpenGLRenderer::Shutdown()
{
	m_private->ShutdownOpenGL();
}


void OpenGLRenderer::SetMachine(Machine* machine)
{
}

