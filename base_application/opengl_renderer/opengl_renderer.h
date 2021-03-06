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
#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

namespace emunisce {

class EmulatedMachine;

class OpenGLRenderer {
public:
	OpenGLRenderer();
	~OpenGLRenderer();

	void Initialize(void* windowHandle);
	void Shutdown();

	void SetMachine(EmulatedMachine* machine);

	int GetLastFrameRendered();

	void SetVsync(bool enabled);

	// Window events

	void Draw();
	void Resize(int newWidth, int newHeight);

private:
	class OpenGLRenderer_Private* m_private;
};

}  // namespace emunisce

#endif
