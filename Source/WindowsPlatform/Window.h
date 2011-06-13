#ifndef WINDOW_H
#define WINDOW_H

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

	void Show();
	void Hide();

	WindowSize GetSize();
	void SetSize(WindowSize size);

	WindowPosition GetPosition();
	void SetPosition(WindowPosition position);

private:

	class Window_Private* m_private;
};

#endif
