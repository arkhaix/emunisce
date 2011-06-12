#ifndef WINDOW_H
#define WINDOW_H

class IWindowMessageListener
{
public:

	virtual void Draw() = 0;
	
	virtual void KeyDown(int key) = 0;
	virtual void KeyUp(int key) = 0;
};

class Window
{
public:

	void Create(int width, int height, const char* title, const char* className);
	void Destroy();

	void Show();
	void Hide();

	void* GetHandle();

	void SubscribeListener(IWindowMessageListener* listener);
	void UnsubscribeListener(IWindowMessageListener* listener);

private:

	class Window_Private* m_private;
};

#endif
