#pragma once
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

struct CriticalData
{
	CriticalData(int idx) : id(idx) {}
	int id;
	std::mutex mut;
};

void deadlock(CriticalData& a, CriticalData& b, int c)
{
	try
	{
		switch (c)
		{
		case 0: // deadlock
		{
			std::lock_guard<std::mutex> g1(a.mut);
			std::cout << std::this_thread::get_id() << " get the first mutex id = " << a.id << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));

			std::cout << "\t" << std::this_thread::get_id() << " trying get the second mutex id = " << b.id << std::endl;
			std::lock_guard<std::mutex> g2(b.mut);
			std::cout << std::this_thread::get_id() << " get the second mutex id = " << b.id << std::endl;
			// do something with a and b
			break;
		}

		case 1: // std::lock with lock_guard
		{
			std::lock(a.mut, b.mut);
			std::lock_guard<std::mutex> g1(a.mut, std::adopt_lock); // lock owners mutex
			std::lock_guard<std::mutex> g2(b.mut, std::adopt_lock);
			// do something here
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			std::cout << std::this_thread::get_id() << " lock done\n";
			break;
		}
		
		case 2: // std::lock with std::unique_lock
		{			
			std::unique_lock<std::mutex> lock1(a.mut, std::defer_lock); // locks has not owner mutex
			std::unique_lock<std::mutex> lock2(b.mut, std::defer_lock);
			std::lock(lock1, lock2);
			// do something here
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			std::cout << std::this_thread::get_id() << " lock done\n";
			break;
		}

		case 3: // scoped_lock
		{
			std::scoped_lock lock(a.mut, b.mut);
			// do something here
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			std::cout << std::this_thread::get_id() << " lock done\n";
		}

		default:
			break;
		}
	}
	catch (std::exception const& e)
	{		
		std::cout << e.what() << std::endl;
	}
	catch (...)
	{
		std::cout << "Unknow exception\n";
	}
}
void testDeadlock(int c)
{
	CriticalData c1(1), c2(2);
	std::thread t1(deadlock, std::ref(c1), std::ref(c2), c);
	std::thread t2(deadlock, std::ref(c2), std::ref(c1), c);
	t1.join();
	t2.join();
}
