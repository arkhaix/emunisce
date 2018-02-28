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

#ifdef EMUNISCE_PLATFORM_WINDOWS
#include "windows.h"
#endif

#include "PlatformDefines.h"

#ifdef EMUNISCE_PLATFORM_LINUX
#include "GL/glew.h"
#endif
#include "GL/gl.h"

#ifdef EMUNISCE_PLATFORM_WINDOWS
#include "glext.h"
#endif

#include "PlatformIncludes.h"

#include "MachineIncludes.h"


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
		Pixels = nullptr;
	}

	OpenGLScreenBuffer(int width, int height)
	{
		Width = width;
		Height = height;
		Pixels = (OpenGLPixel*)malloc(width * height * sizeof(OpenGLPixel));
	}

	~OpenGLScreenBuffer()
	{
		if(Pixels != nullptr)
		{
			free((void*)Pixels);
			Pixels = nullptr;

			Width = -1;
			Height = -1;
		}
	}
};

class OpenGLRenderer_Private
{
public:

	IEmulatedMachine* _Machine;
	IEmulatedDisplay* _Display;

	bool _NeedsShutdown;

#ifdef EMUNISCE_PLATFORM_WINDOWS
	HWND _WindowHandle;
	HDC _DeviceContext;
	HGLRC _RenderContext;

	HDC _PreservedDeviceContext;
	HGLRC _PreservedRenderContext;

	typedef void (APIENTRY *TwglSwapIntervalEXT)(int mode);
	TwglSwapIntervalEXT wglSwapIntervalEXT;

	typedef void (APIENTRY *TglGenBuffersARB)(GLsizei n, GLuint* buffers);
	typedef void (APIENTRY *TglBindBufferARB)(GLenum target, GLuint id);
	typedef void (APIENTRY *TglBufferDataARB)(GLenum target, int size, const GLvoid *data, GLenum usage);
	typedef void (APIENTRY *TglDeleteBuffersARB)(int size, GLuint* buffers);
	typedef GLvoid* (APIENTRY *TglMapBufferARB)(GLenum target, GLenum access);
	typedef GLboolean (APIENTRY *TglUnmapBufferARB)(GLenum target);

	TglGenBuffersARB glGenBuffersARB;
	TglBindBufferARB glBindBufferARB;
	TglBufferDataARB glBufferDataARB;
	TglDeleteBuffersARB glDeleteBuffersARB;
	TglMapBufferARB glMapBufferARB;
	TglUnmapBufferARB glUnmapBufferARB;
#endif

	OpenGLScreenBuffer* _ScreenBuffer;
	int _LastFrameRendered;
	int _LastFrameDrawn;

	GLuint _ScreenTexture;

	int _ClientWidth;
	int _ClientHeight;

	bool _PboSupported;
	GLuint _PixelBufferObject;


	OpenGLRenderer_Private()
	{
		_Machine = nullptr;
		_Display = nullptr;

		_NeedsShutdown = false;

#ifdef EMUNISCE_PLATFORM_WINDOWS
		_WindowHandle = nullptr;
		_DeviceContext = nullptr;
		_RenderContext = nullptr;

		_PreservedDeviceContext = nullptr;
		_PreservedRenderContext = nullptr;

		wglSwapIntervalEXT = nullptr;

		glGenBuffersARB = nullptr;
		glBindBufferARB = nullptr;
		glBufferDataARB = nullptr;
		glDeleteBuffersARB = nullptr;
		glMapBufferARB = nullptr;
		glUnmapBufferARB = nullptr;
#endif

		_ScreenBuffer = nullptr;
		_LastFrameRendered = -1;
		_LastFrameDrawn = -1;

		_ScreenTexture = 0;

		_ClientWidth = 1;
		_ClientHeight = 1;

		_PboSupported = false;
		_PixelBufferObject = 0;
	}

	bool IsExtensionSupported(const char *extension)
	{
		//This function is modified from NeHe's tutorial #46.
		//http://nehe.gamedev.net/data/lessons/lesson.asp?lesson=46

		const size_t extlen = strlen(extension);
		const char *supported = nullptr;

#ifdef EMUNISCE_PLATFORM_WINDOWS
		// Try To Use wglGetExtensionStringARB On Current DC, If Possible
		PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

		if (wglGetExtString)
			supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
#endif

		if(supported != nullptr)
		{
			// Begin Examination At Start Of String, Increment By 1 On False Match
			for (const char* p = supported; ; p++)
			{
				// Advance p Up To The Next Possible Match
				p = strstr(p, extension);

				if (p == nullptr)
					break;						// No Match

				// Make Sure That Match Is At The Start Of The String Or That
				// The Previous Char Is A Space, Or Else We Could Accidentally
				// Match "wglFunkywglExtension" With "wglExtension"

				// Also, Make Sure That The Following Character Is Space Or nullptr
				// Or Else "wglExtensionTwo" Might Match "wglExtension"
				if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
					return true;						// Match
			}
		}

		//Not found in wgl extensions.  Try standard opengl extensions.
		supported = (char*)glGetString(GL_EXTENSIONS);
		if (supported != nullptr)
		{
			// Begin Examination At Start Of String, Increment By 1 On False Match
			for (const char* p = supported; ; p++)
			{
				// Advance p Up To The Next Possible Match
				p = strstr(p, extension);

				if (p == nullptr)
					break;						// No Match

				// Make Sure That Match Is At The Start Of The String Or That
				// The Previous Char Is A Space, Or Else We Could Accidentally
				// Match "wglFunkywglExtension" With "wglExtension"

				// Also, Make Sure That The Following Character Is Space Or nullptr
				// Or Else "wglExtensionTwo" Might Match "wglExtension"
				if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
					return true;						// Match
			}
		}

		//No dice
		return false;
	}

	void InitializeOpenGL()
	{
		_NeedsShutdown = true;

#ifdef EMUNISCE_PLATFORM_WINDOWS
		if(_WindowHandle != nullptr)
		{
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
		}
#elif EMUNISCE_PLATFORM_LINUX
        glewInit();
#endif

		glShadeModel(GL_FLAT);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

		//Enable v-sync if supported
#ifdef EMUNISCE_PLATFORM_WINDOWS
		if(IsExtensionSupported("WGL_EXT_swap_control") == true)
		{
			wglSwapIntervalEXT = (TwglSwapIntervalEXT)wglGetProcAddress("wglSwapIntervalEXT");
			SetVsync(true);
		}
#elif EMUNISCE_PLATFORM_LINUX
        // wtf glew
#endif

#ifdef EMUNISCE_PLATFORM_WINDOWS
		//Enable PBO method if supported
		if(IsExtensionSupported("GL_ARB_pixel_buffer_object") == true)
		{
			_PboSupported = true;

			glGenBuffersARB = (TglGenBuffersARB)wglGetProcAddress("glGenBuffersARB");
			glBindBufferARB = (TglBindBufferARB)wglGetProcAddress("glBindBufferARB");
			glBufferDataARB = (TglBufferDataARB)wglGetProcAddress("glBufferDataARB");
			glDeleteBuffersARB = (TglDeleteBuffersARB)wglGetProcAddress("glDeleteBuffersARB");
			glMapBufferARB = (TglMapBufferARB)wglGetProcAddress("glMapBufferARB");
			glUnmapBufferARB = (TglUnmapBufferARB)wglGetProcAddress("glUnmapBufferARB");

			if(glGenBuffersARB == nullptr || glBindBufferARB == nullptr || glBufferDataARB == nullptr ||
				glDeleteBuffersARB == nullptr || glMapBufferARB == nullptr || glUnmapBufferARB == nullptr)
				_PboSupported = false;
		}

		if(_WindowHandle != nullptr)
			RestorePreservedContext();
#elif EMUNISCE_PLATFORM_LINUX
        if(GLEW_ARB_pixel_buffer_object)
            _PboSupported = true;
#endif
	}

	void ShutdownOpenGL()
	{
		if(_NeedsShutdown == false)
			return;

		_NeedsShutdown = false;

#ifdef EMUNISCE_PLATFORM_WINDOWS
		if(_WindowHandle != nullptr)
		{
			wglDeleteContext(_RenderContext);
			ReleaseDC(_WindowHandle, _DeviceContext);
		}
#endif
	}

	void PreserveExistingContext()
	{
#ifdef EMUNISCE_PLATFORM_WINDOWS
		//_PreservedDeviceContext = wglGetCurrentDC();
		//_PreservedRenderContext = wglGetCurrentContext();
#endif
	}

	void RestorePreservedContext()
	{
#ifdef EMUNISCE_PLATFORM_WINDOWS
		//wglMakeCurrent(_PreservedDeviceContext, _PreservedRenderContext);
#endif
	}

	void SetVsync(bool enabled)
	{
#ifdef EMUNISCE_PLATFORM_WINDOWS
		if(wglSwapIntervalEXT != nullptr)
		{
			int mode = 1;
			if(enabled == false)
				mode = 0;

			wglSwapIntervalEXT(mode);
		}
#endif
	}

	void UpdateTexture()
	{
		if(_Display == nullptr)
			return;

		if(_Display->GetScreenBufferCount() == _LastFrameRendered)
			return;

		_LastFrameRendered = _Display->GetScreenBufferCount();


		ScreenBuffer* displayScreen = _Display->GetStableScreenBuffer();

		int displayWidth = displayScreen->GetWidth();
		int displayHeight = displayScreen->GetHeight();
		int displayDataSize = displayWidth * displayHeight * sizeof(DisplayPixel);

		bool resizeTexture = false;
		if(_ScreenBuffer == nullptr || _ScreenBuffer->Width != displayWidth || _ScreenBuffer->Height != displayHeight)
		{
			if(_ScreenBuffer != nullptr)
				delete _ScreenBuffer;

			_ScreenBuffer = new OpenGLScreenBuffer(displayWidth, displayHeight);
			resizeTexture = true;
		}

		//Normally, texture format conversion goes here, but right now displayScreen->GetPixels
		// outputs the native texture format, so I'm passing it directly into the gl functions.
		/*
		int numPixels = displayWidth * displayHeight;
		memcpy((void*)_ScreenBuffer->Pixels, (void*)displayScreen->GetPixels(), numPixels * sizeof(DisplayPixel));
		*/

		//PixelBufferObject method
		if(_PboSupported == true)
		{
			if(resizeTexture == true)
			{
				//Free the old resources

				//Texture
				if(_ScreenTexture != 0)
					glDeleteTextures(1, &_ScreenTexture);

				//PBO
				if(_PixelBufferObject != 0)
					glDeleteBuffersARB(1, &_PixelBufferObject);


				//Allocate new resources

				//Texture
				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				glGenTextures(1, &_ScreenTexture);
				glBindTexture(GL_TEXTURE_2D, _ScreenTexture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, displayWidth, displayHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
				glBindTexture(GL_TEXTURE_2D, 0);

				//PBO
				glGenBuffersARB(1, &_PixelBufferObject);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _PixelBufferObject);
				glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, displayDataSize, nullptr, GL_STREAM_DRAW_ARB);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
			}

			//Bind things
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, _PixelBufferObject);

			//Map the buffer so we can modify it
			glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, displayDataSize, nullptr, GL_STREAM_DRAW_ARB);
			GLubyte* mappedData = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);

			//Update the buffer
			if(mappedData)
			{
				//memcpy((void*)mappedData, (void*)_ScreenBuffer->Pixels, displayDataSize);
				memcpy((void*)mappedData, (void*)displayScreen->GetPixels(), displayDataSize);
				glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
			}

			//Copy data from the pbo to the texture
			glBindTexture(GL_TEXTURE_2D, _ScreenTexture);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, displayWidth, displayHeight, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
			glBindTexture(GL_TEXTURE_2D, 0);

			//We're done. Unbind the buffer.
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		}

		//TexImage2D method
		else
		{
			if(resizeTexture == true)
			{
				if(_ScreenTexture != 0)
					glDeleteTextures(1, &_ScreenTexture);

				glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
				glPixelStorei(GL_PACK_ALIGNMENT, 1);
				glGenTextures(1, &_ScreenTexture);
				glBindTexture(GL_TEXTURE_2D, _ScreenTexture);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, displayWidth, displayHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			glBindTexture(GL_TEXTURE_2D, _ScreenTexture);

			//glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, displayWidth, displayHeight, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, (GLvoid*)_ScreenBuffer->Pixels);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, displayWidth, displayHeight, GL_BGRA_EXT, GL_UNSIGNED_INT_8_8_8_8_REV, (GLvoid*)displayScreen->GetPixels());

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void RenderToDisplay()
	{
		if(_ScreenBuffer == nullptr || _ScreenBuffer->Width <= 0 || _ScreenBuffer->Height <= 0)
			return;

		if(_LastFrameDrawn == _LastFrameRendered)
			return;

		_LastFrameDrawn = _LastFrameRendered;

		//Make sure we're covering the whole window (even after resize)
		glViewport(0, 0, _ClientWidth, _ClientHeight);

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
			glVertex2f((GLfloat)_ClientWidth, -1.0);

			glTexCoord2f(0.0, 0.0);
			glVertex2f(-1.0, (GLfloat)_ClientHeight);

			glTexCoord2f(1.0, 0.0);
			glVertex2f((GLfloat)_ClientWidth, (GLfloat)_ClientHeight);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		//Clean up the matrix stack
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();

		//Display the rendered frame to the window
#ifdef EMUNISCE_PLATFORM_WINDOWS
		if(_DeviceContext != nullptr)
			SwapBuffers(_DeviceContext);
#endif
	}


	void Draw()
	{
		PreserveExistingContext();
#ifdef EMUNISCE_PLATFORM_WINDOWS
		if(_DeviceContext != nullptr && _RenderContext != nullptr)
			wglMakeCurrent(_DeviceContext, _RenderContext);
#endif
#ifdef EMUNISCE_PLATFORM_LINUX
        static bool glInitialized = false;
        if(glInitialized == false)
        {
            InitializeOpenGL();
            glInitialized = true;
        }
#endif

		UpdateTexture();
		RenderToDisplay();

		RestorePreservedContext();
	}

	void Resize(int newWidth, int newHeight)
	{
		_ClientWidth = newWidth;
		_ClientHeight = newHeight;
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


void OpenGLRenderer::Initialize(void* windowHandle)
{
#ifdef EMUNISCE_PLATFORM_WINDOWS
	m_private->_WindowHandle = (HWND)windowHandle;
#endif

	m_private->InitializeOpenGL();
}

void OpenGLRenderer::Shutdown()
{
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


void OpenGLRenderer::SetVsync(bool enabled)
{
	m_private->SetVsync(enabled);
}


void OpenGLRenderer::Draw()
{
	m_private->Draw();
}

void OpenGLRenderer::Resize(int newWidth, int newHeight)
{
	m_private->Resize(newWidth, newHeight);
}
