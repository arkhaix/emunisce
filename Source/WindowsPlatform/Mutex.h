#ifndef MUTEX_H
#define MUTEX_H

class Mutex
{
public:

	Mutex();
	~Mutex();

	void Lock();
	void Unlock();

private:

	class Mutex_Private* m_private;
};

#endif
