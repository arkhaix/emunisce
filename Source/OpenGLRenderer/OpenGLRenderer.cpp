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


class OpenGLRenderer_Private : public IWindowMessageListener
{
public:

	Phoenix* _Phoenix;

	Machine* _Machine;
	Display* _Display;

	HWND _WindowHandle;
	HDC _DeviceContext;
	HGLRC _RenderContext;
	bool _NeedsShutdown;

	HDC _PreservedDeviceContext;
	HGLRC _PreservedRenderContext;

	OpenGLRenderer_Private()
	{
		_Phoenix = NULL;

		_Machine = NULL;
		_Display = NULL;

		_WindowHandle = NULL;
		_DeviceContext = NULL;
		_RenderContext = NULL;
		_NeedsShutdown = false;

		_PreservedDeviceContext = NULL;
		_PreservedRenderContext = NULL;
	}

	void InitializeOpenGL()
	{
		if(_WindowHandle == NULL)
			return;

		_NeedsShutdown = true;

		_DeviceContext = GetDC(_WindowHandle);

		PIXELFORMATDESCRIPTOR desiredPixelFormat;
		ZeroMemory(&desiredPixelFormat, sizeof(desiredPixelFormat));
		desiredPixelFormat.nSize = sizeof(desiredPixelFormat);

		desiredPixelFormat.nVersion = 1;
		desiredPixelFormat.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		desiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
		desiredPixelFormat.cColorBits = 24;
		desiredPixelFormat.cDepthBits = 16;
		desiredPixelFormat.iLayerType = PFD_MAIN_PLANE;

		int pixelFormat;
		pixelFormat = ChoosePixelFormat(_DeviceContext, &desiredPixelFormat);
		SetPixelFormat(_DeviceContext, pixelFormat, &desiredPixelFormat);

		_RenderContext = wglCreateContext(_DeviceContext);

		PreserveExistingContext();
		wglMakeCurrent(_DeviceContext, _RenderContext);

		glShadeModel(GL_SMOOTH);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

		RestorePreservedContext();
	}

	void ShutdownOpenGL()
	{
		if(_NeedsShutdown == false)
			return;

		_NeedsShutdown = false;
	
		wglDeleteContext(_RenderContext);
		ReleaseDC(_WindowHandle, _DeviceContext);
	}

	void PreserveExistingContext()
	{
		_PreservedDeviceContext = wglGetCurrentDC();
		_PreservedRenderContext = wglGetCurrentContext();
	}

	void RestorePreservedContext()
	{
		wglMakeCurrent(_PreservedDeviceContext, _PreservedRenderContext);
	}

	void UpdateTexture()
	{
	}

	void RenderToWindow()
	{
		if(_Phoenix->GetWindow() == NULL)
			return;

		PreserveExistingContext();
		wglMakeCurrent(_DeviceContext, _RenderContext);

		//Make sure we're covering the whole window (even after resize)
		WindowSize windowSize = _Phoenix->GetWindow()->GetSize();
		glViewport(0, 0, windowSize.width, windowSize.height);

		//Setup 2D projection
		int viewPort[4];

		glGetIntegerv(GL_VIEWPORT, viewPort);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();

		glOrtho(0, viewPort[2], 0, viewPort[3], -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		//2D projection is done.  Now clear the screen so we can start fresh.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		//Do more stuff...
		glBegin(GL_TRIANGLES);
			glColor3ub(255, 0, 0);
			glVertex2d(0, 0);
			glColor3ub(0, 255, 0);
			glVertex2d(100,0);
			glColor3ub(0, 0, 255);
			glVertex2d(50, 50);
		glEnd();

		//Done doing stuff
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();   
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		RestorePreservedContext();
	}


	// IWindowMessageListener
	
	void Closed()
	{
		ShutdownOpenGL();
	}


	void Draw()
	{
		UpdateTexture();
		RenderToWindow();
	}


	void Resize()
	{
		
	}

	
	void KeyDown(int key)
	{
	}

	void KeyUp(int key)
	{
	}
};


OpenGLRenderer::OpenGLRenderer()
{
	m_private = new OpenGLRenderer_Private();
}

OpenGLRenderer::~OpenGLRenderer()
{
	delete m_private;
}


void OpenGLRenderer::Initialize(Phoenix* phoenix)
{
	m_private->_Phoenix = phoenix;
	m_private->_WindowHandle = (HWND)phoenix->GetWindow()->GetHandle();

	m_private->InitializeOpenGL();

	phoenix->GetWindow()->SubscribeListener(m_private);
}

void OpenGLRenderer::Shutdown()
{
	m_private->_Phoenix->GetWindow()->UnsubscribeListener(m_private);

	m_private->ShutdownOpenGL();
}


void OpenGLRenderer::SetMachine(Machine* machine)
{
	//todo: lock things
	m_private->_Machine = machine;
	m_private->_Display = machine->GetDisplay();
}

