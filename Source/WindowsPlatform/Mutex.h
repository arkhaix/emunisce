#ifndef MUTEX_H
#define MUTEX_H

class Mutex
{
public:

	Mutex();
	~Mutex();

	void Acquire();
	void Release();

private:

	class Mutex_Private* m_private;
};

class ScopedMutex
{
public:

	Mutex& m_mutex;

	ScopedMutex(Mutex& mutex)
	: m_mutex(mutex)
	{
		m_mutex.Acquire();
	}

	~ScopedMutex()
	{
		m_mutex.Release();
	}
};

#endif
