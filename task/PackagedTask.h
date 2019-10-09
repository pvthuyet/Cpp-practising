#pragma once
#include <future>
#include <deque>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

std::mutex coutMutex;

struct SumUp
{
	long long int operator()(int beg, int end)
	{
		long long int sum{ 0 };
		for (int i = beg; i < end; ++i)
			sum += i;
		return sum;
	}
};

void testPackagedTask()
{
	std::cout << "******** How to use package task *********\n";
	SumUp su1, su2, su3, su4;

	// wrap the task
	std::packaged_task<long long int(int, int)> tsk1(su1), tsk2(su2), tsk3(su3), tsk4(su4);
	
	// create future
	std::future<long long int> fu1 = tsk1.get_future();
	std::future<long long int> fu2 = tsk2.get_future();
	std::future<long long int> fu3 = tsk3.get_future();
	std::future<long long int> fu4 = tsk4.get_future();
	
	// push task to container
	std::deque<std::packaged_task<long long int(int, int)> > allTask;
	
	allTask.emplace_back(std::move(tsk1));
	allTask.emplace_back(std::move(tsk2));
	allTask.emplace_back(std::move(tsk3));
	allTask.emplace_back(std::move(tsk4));

	int begin{ 1 };
	int increment{ 2500 };
	int end = begin + increment;

	//perform
	while (!allTask.empty())
	{
		auto myTask = std::move(allTask.front());
		allTask.pop_front();
		std::thread t(std::move(myTask), begin, end);
		begin = end;
		end += increment;
		t.detach();
	}

	auto sum = fu1.get() + fu2.get() + fu3.get() + fu4.get();
	std::cout << "sum of 0 ... 10000 = " << sum << std::endl;
}

/******** How to reuse future *********/
void calcProducts(std::packaged_task<long long int(int, int)>& task, std::vector<std::pair<int, int> > const &pairs)
{
	for (auto& p : pairs)
	{
		auto fu = task.get_future();
		task(p.first, p.second);
		std::cout << p.first << " * " << p.second << " = " << fu.get() << std::endl;
		task.reset();
	}
}
void testReuse()
{
	std::cout << "******** How to reuse future *********\n";
	std::vector<std::pair<int, int> > allPairs;
	allPairs.emplace_back(std::make_pair(1, 2));
	allPairs.emplace_back(std::make_pair(2, 3));
	allPairs.emplace_back(std::make_pair(3, 4));
	allPairs.emplace_back(std::make_pair(4, 5));

	std::packaged_task<long long int(int, int)> task([](int fir, int sec) {
		return fir * sec;
	});

	// main thread
	calcProducts(task, allPairs);

	std::cout << std::endl;
	// create thread
	std::thread t(calcProducts, std::ref(task), allPairs);
	t.join();
}

/******** Promise and future *********/
void product(std::promise<long long int>&& pro, int a, int b)
{
	pro.set_value(a*b);
}

struct divtest
{
	void operator()(std::promise<long long int>&& pro, int a, int b)
	{
		try
		{
			if (b == 0)
			{
				std::string errMsg = std::string("Illegal division by zero: ") +
					std::string("Illegal division by zero: ");
				throw std::runtime_error(errMsg);
			}
			pro.set_value(a / b);
		}
		catch (...)
		{
			pro.set_exception(std::current_exception());
		}
	}
};

void testPromise()
{
	std::cout << "******** Promise and future *********\n";
	int a(20), b(10);	
	// promise
	std::promise<long long int> proPromise;
	std::promise<long long int> divPromise;
	// future
	std::future<long long int> proRes = proPromise.get_future();
	std::future<long long int> divRes = divPromise.get_future();
	// calc
	std::thread proThread(product, std::move(proPromise), a, b);
	divtest div;
	std::thread divThread(div, std::move(divPromise), a, b);

	std::cout << a << " * " << b << " = " << proRes.get() << std::endl;
	std::cout << a << " / " << b << " = " << divRes.get() << std::endl;
	proThread.join();
	divThread.join();
}

/******* Waiting for the promise **********************************/
void getAnswer(std::promise<int> pro)
{
	using namespace std::chrono_literals;
	std::this_thread::sleep_for(3s);
	pro.set_value(69);
}
void testWaitPromise()
{
	std::cout << "******** Waiting for the promise *********\n";
	using namespace std::chrono_literals;
	std::promise<int> ansPro;
	std::future<int> fut = ansPro.get_future();
	std::thread t(getAnswer, std::move(ansPro));
	std::future_status stat{};
	do 
	{
		stat = fut.wait_for(0.2s);
		std::cout << "...doing something else\n";

	} while (stat != std::future_status::ready);
	std::cout << "The answer is " << fut.get() << std::endl;
	t.join();
}
/******* Share future **********************************/
struct requestor
{
	void operator()(std::shared_future<long long int> shaFut)
	{
		using namespace std::chrono_literals;
		std::lock_guard<std::mutex> lk(coutMutex);
		try
		{
			std::cout << std::this_thread::get_id() << "\t20/10 = " << shaFut.get() << std::endl;
		}
		catch (std::runtime_error& e)
		{
			std::cout << e.what() << std::endl;
		}
		std::this_thread::sleep_for(0.5s);
	}
};
void testSharedFuture()
{
	std::cout<< std::boolalpha;
	std::cout << "******* Share future **********************************\n";
	std::promise<long long int> divPro;
	std::future<long long int> divRes = divPro.get_future();
	std::cout << "Future is valid: " << divRes.valid() << std::endl;
	
	divtest div;
	std::thread divThr(div, std::move(divPro), 20, 0);
	std::cout << "Future is valid: " << divRes.valid() << std::endl;
	std::shared_future<long long int> sharedRes = divRes.share();
	std::cout << "Future is valid: " << divRes.valid() << std::endl;

	requestor req;
	std::thread t1(req, sharedRes);
	std::thread t2(req, sharedRes);
	std::thread t3(req, sharedRes);
	std::thread t4(req, sharedRes);
	std::thread t5(req, sharedRes);

	divThr.join();
	t1.join();
	t2.join();
	t3.join();
	t4.join();
	t5.join();
}
namespace cond
{
	std::mutex mut;
	std::condition_variable cv;
	std::atomic_bool dataReady(false);

	void waitForWork()
	{
		std::cout << "Worker waiting for work.\n";
		std::unique_lock lk(mut);
		cv.wait(lk, [] {return dataReady.load(std::memory_order_relaxed); });
		std::cout << "work done\n";
	}
	void setReady()
	{
		std::lock_guard<std::mutex> lk(mut);
		dataReady.store(true, std::memory_order_relaxed); // event amtomic must modify under lock
		std::cout << "Send data is ready.\n";
		cv.notify_one();
	}
	void test()
	{
		std::cout << "******* Condition variable test ******************\n";
		using namespace std::chrono_literals;
		std::thread t1(waitForWork);
		std::this_thread::sleep_for(2s);
		std::thread t2(setReady);
		t1.join();
		t2.join();
	}
}

namespace profut
{
	void waitForWork(std::future<void> && fut)
	{
		std::cout << "Worker: Waiting for work." << std::endl;
		fut.wait();
		std::cout << "work done\n";
	}
	void setReady(std::promise<void> &&pro)
	{
		std::cout << "Send data is ready.\n";
		pro.set_value();
	}
	void test()
	{
		std::cout << "******* promise future test ******************\n";
		using namespace std::chrono_literals;
		std::promise<void> pro;
		std::future<void> fut = pro.get_future();
		std::thread t1(waitForWork, std::move(fut));
		std::this_thread::sleep_for(2s);
		std::thread t2(setReady, std::move(pro));
		t1.join();
		t2.join();
	}
}