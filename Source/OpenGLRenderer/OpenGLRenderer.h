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
#ifndef OPENGLRENDERER_H
#define OPENGLRENDERER_H

class Phoenix;
class Machine;

class OpenGLRenderer
{
public:

	OpenGLRenderer();
	~OpenGLRenderer();

	void Initialize(Phoenix* phoenix);
	void Shutdown();

	void SetMachine(Machine* machine);

private:

	class OpenGLRenderer_Private* m_private;
};

#endif
