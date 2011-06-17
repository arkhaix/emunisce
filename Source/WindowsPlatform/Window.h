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
#ifndef WINDOW_H
#define WINDOW_H


namespace Emunisce
{

class IWindowMessageListener
{
public:

	virtual void Closed() = 0;

	virtual void Draw() = 0;

	virtual void Resize() = 0;
	
	virtual void KeyDown(int key) = 0;
	virtual void KeyUp(int key) = 0;
};

struct WindowSize
{
	int width;
	int height;
};

struct WindowPosition
{
	int x;
	int y;
};

class Window
{
public:

	Window();
	~Window();

	void Create(int width = 640, int height = 480, const char* title = "", const char* className = "GenericWindow");
	void Destroy();

	void* GetHandle();

	void SubscribeListener(IWindowMessageListener* listener);
	void UnsubscribeListener(IWindowMessageListener* listener);

	void PumpMessages();
	void RequestExit();

	void Show();
	void Hide();

	WindowSize GetSize();
	void SetSize(WindowSize size);

	WindowPosition GetPosition();
	void SetPosition(WindowPosition position);

private:

	class Window_Private* m_private;
};

}	//namespace Emunisce

#endif
