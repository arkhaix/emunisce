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
#include "OpenGLRenderer.h"
using namespace Emunisce;

#include "windows.h"

#include "gl/gl.h"

#include "PlatformIncludes.h"

#include "MachineIncludes.h"

#include "../Emunisce/Emunisce.h"



namespace Emunisce
{

union OpenGLPixel
{
	u32 RGBA32;

	GLubyte RGBA[4];

	struct
	{
		GLubyte R;
		GLubyte G;
		GLubyte B;
		GLubyte A;
	} Elements;

	inline OpenGLPixel& operator=(const DisplayPixel& rhs)
	{
		RGBA32 = (u32)rhs;
		return *this;
	}

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

	EmunisceApplication* _Phoenix;

	IEmulatedMachine* _Machine;
	IEmulatedDisplay* _Display;

	HWND _WindowHandle;
	HDC _DeviceContext;
	HGLRC _RenderContext;
	bool _NeedsShutdown;

	HDC _PreservedDeviceContext;
	HGLRC _PreservedRenderContext;

	OpenGLScreenBuffer* _ScreenBuffer;
	int _LastFrameRendered;

	GLuint _ScreenTexture;

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

		_ScreenTexture = 0;
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
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

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


		ScreenBuffer* displayScreen = _Display->GetStableScreenBuffer();

		int displayWidth = displayScreen->GetWidth();
		int displayHeight = displayScreen->GetHeight();

		if(_ScreenBuffer == NULL || _ScreenBuffer->Width != displayWidth || _ScreenBuffer->Height != displayHeight)
		{
			if(_ScreenBuffer != NULL)
				delete _ScreenBuffer;

			_ScreenBuffer = new OpenGLScreenBuffer(displayWidth, displayHeight);
		}

		for(int y=0;y<displayHeight;y++)
		{
			for(int x=0;x<displayWidth;x++)
			{
				DisplayPixel pixelValue = displayScreen->GetPixels()[y * displayWidth + x];

				_ScreenBuffer->Pixels[y * displayWidth + x] = pixelValue;
			}
		}

		PreserveExistingContext();
		wglMakeCurrent(_DeviceContext, _RenderContext);

		if(_ScreenTexture != 0)
		{
			glDeleteTextures(1, &_ScreenTexture);
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glGenTextures(1, &_ScreenTexture);
		glBindTexture(GL_TEXTURE_2D, _ScreenTexture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		//todo: power of 2 problems?
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, displayWidth, displayHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*)_ScreenBuffer->Pixels);

		RestorePreservedContext();
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

		//2D projection is done.  Clear the screen.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glLoadIdentity();

		//Render
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, _ScreenTexture);
		glBegin(GL_TRIANGLE_STRIP);
			glTexCoord2f(0.0, 1.0);
			glVertex2f(-1.0,-1.0);

			glTexCoord2f(1.0, 1.0);
			glVertex2f((GLfloat)clientWidth, -1.0);

			glTexCoord2f(0.0, 0.0);
			glVertex2f(-1.0, (GLfloat)clientHeight);

			glTexCoord2f(1.0, 0.0);
			glVertex2f((GLfloat)clientWidth, (GLfloat)clientHeight);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		
		//Clean up the matrix stack
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();   
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

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

}	//namespace Emunisce


OpenGLRenderer::OpenGLRenderer()
{
	m_private = new OpenGLRenderer_Private();
}

OpenGLRenderer::~OpenGLRenderer()
{
	delete m_private;
}


void OpenGLRenderer::Initialize(EmunisceApplication* phoenix)
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


void OpenGLRenderer::SetMachine(IEmulatedMachine* machine)
{
	//todo: lock things
	m_private->_Machine = machine;
	m_private->_Display = machine->GetDisplay();
}


int OpenGLRenderer::GetLastFrameRendered()
{
	return m_private->_LastFrameRendered;
}
