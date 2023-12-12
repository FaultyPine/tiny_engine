#ifndef THREADSSAFE_QUEUE_H
#define THREADSSAFE_QUEUE_H

#include <atomic> 
#include <mutex>
#include "mem/tiny_arena.h"
#include "tiny_log.h"

// Fixed size very simple thread safe ring buffer
template <typename T, size_t _capacity>
class MutexQueue
{
public:

	MutexQueue()
	{
		TMEMSET(data, 0, sizeof(T) * _capacity);
		head = 0; tail = 0;
	}

	// Push an item to the end if there is free space
	//	Returns true if succesful
	//	Returns false if there is not enough space
	inline bool push_back(const T& item)
	{
		bool result = false;
		lock.lock();
		size_t next = (head + 1) % _capacity;
		if (next != tail)
		{
			data[head] = item;
			head = next;
			result = true;
		}
		lock.unlock();
		return result;
	}

	// Get an item if there are any
	//	Returns true if succesful
	//	Returns false if there are no items
	inline bool pop_front(T& item)
	{
		bool result = false;
		lock.lock();
		if (tail != head)
		{
			item = data[tail];
			tail = (tail + 1) % _capacity;
			result = true;
		}
		lock.unlock();
		return result;
	}

	inline u32 capacity()
	{
		return ARRAY_SIZE(data);
	}

	inline u32 size()
	{
		if (head < tail)
		{
			return (_capacity - tail) + head;
		}
		else if (head > tail)
		{
			return head - tail;
		}
		return 0; // head = tail means 0 elements
	}

	inline void clear()
	{
		lock.lock();
		TMEMSET(data, 0, sizeof(T) * _capacity);
		head = 0; tail = 0;
		lock.unlock();
	}

	inline const T& get(u32 index)
	{
		if (index >= 0 && index < _capacity)
		{
			return data[index];
		}
		TINY_ASSERT(false && "out of bounds access in mutexqueue");
		return data[0];
	}

	std::mutex lock; // this just works better than a spinlock here (on windows)
private:
	T data[_capacity];
	size_t head = 0;
	size_t tail = 0;
};

#endif