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


union OpenGLPixel
{
	GLubyte RGB[4];

	struct
	{
		GLubyte R;
		GLubyte G;
		GLubyte B;
		GLubyte A;
	} Elements;

	OpenGLPixel(GLubyte r, GLubyte g, GLubyte b, GLubyte a=255)
	{
		Elements.R = r;
		Elements.G = g;
		Elements.B = b;
		Elements.A = a;
	}
};

struct OpenGLScreenBuffer
{
	int Width;
	int Height;
	OpenGLPixel* Pixels;

	OpenGLScreenBuffer()
	{
		Width = -1;
		Height = -1;
		Pixels = NULL;
	}

	OpenGLScreenBuffer(int width, int height)
	{
		Width = width;
		Height = height;
		Pixels = (OpenGLPixel*)malloc(width * height * sizeof(OpenGLPixel));
	}

	~OpenGLScreenBuffer()
	{
		if(Pixels != NULL)
		{
			free((void*)Pixels);
			Pixels = NULL;

			Width = -1;
			Height = -1;
		}
	}
};

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

	OpenGLScreenBuffer* _ScreenBuffer;
	int _LastFrameRendered;

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

		_ScreenBuffer = NULL;
		_LastFrameRendered = -1;
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

		glShadeModel(GL_FLAT);
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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
		if(_Display == NULL)
			return;

		if(_Display->GetScreenBufferCount() == _LastFrameRendered)
			return;

		_LastFrameRendered = _Display->GetScreenBufferCount();

		ScreenResolution displayResolution = _Display->GetScreenResolution();
		if(_ScreenBuffer == NULL || _ScreenBuffer->Width != displayResolution.width || _ScreenBuffer->Height != displayResolution.height)
		{
			if(_ScreenBuffer != NULL)
				delete _ScreenBuffer;

			_ScreenBuffer = new OpenGLScreenBuffer(displayResolution.width, displayResolution.height);
		}

		OpenGLPixel palette[4] =
		{
			OpenGLPixel(255, 255, 255),
			OpenGLPixel(170, 170, 170),
			OpenGLPixel(85, 85, 85),
			OpenGLPixel(0, 0, 0)
		};

		ScreenBuffer displayScreen = _Display->GetStableScreenBuffer();

		for(int y=0;y<displayResolution.height;y++)
		{
			for(int x=0;x<displayResolution.width;x++)
			{
				u8 pixelValue = displayScreen.GetPixel(x,y);
				if(pixelValue < 0 || pixelValue > 3)
					continue;

				_ScreenBuffer->Pixels[ (y * displayResolution.width) + x ] = palette[ displayScreen.GetPixel(x,y) ];
			}
		}
	}

	void RenderToWindow()
	{
		if(_ScreenBuffer == NULL || _ScreenBuffer->Width <= 0 || _ScreenBuffer->Height <= 0)
			return;

		if(_Phoenix->GetWindow() == NULL)
			return;

		PreserveExistingContext();
		wglMakeCurrent(_DeviceContext, _RenderContext);

		//Make sure we're covering the whole window (even after resize)
		RECT clientRect;
		GetClientRect(_WindowHandle, &clientRect);
		int clientWidth = clientRect.right - clientRect.left;
		int clientHeight = clientRect.bottom - clientRect.top;
		glViewport(0, 0, clientWidth, clientHeight);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0.0, (GLdouble)clientWidth, 0.0, (GLdouble)clientHeight, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Render
		glRasterPos2i(0, 0);
		glDrawPixels(_ScreenBuffer->Width, _ScreenBuffer->Height, GL_RGBA, GL_UNSIGNED_BYTE, _ScreenBuffer->Pixels);
		glFlush();
		
		//Display the rendered frame to the window
		SwapBuffers(_DeviceContext);

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

