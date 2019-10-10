#pragma once
#include <map>
#include <string>
#include <shared_mutex>
#include <string_view>
#include <iostream>
#include <chrono>

std::map<std::string, int> teleBook{
{ "Dijkstra", 1972} ,
{ "Scott", 1976},
{ "Ritchie", 1983}
};
std::shared_timed_mutex teleBookMutex;

void addToTeleBook(const std::string& na, int tele)
{
	using namespace std::chrono_literals;
	std::lock_guard<std::shared_timed_mutex> lk(teleBookMutex);
	std::cout <<std::this_thread::get_id() << "\tStart update " << na << std::endl;
	std::this_thread::sleep_for(1s);
	teleBook[na] = tele;
	std::cout << "end update\n";
}

void printNumber(const std::string& na)
{
	using namespace std::chrono_literals;
	std::shared_lock<std::shared_timed_mutex> writeLock(teleBookMutex);
	auto searchEntry = teleBook.find(na);
	if (searchEntry != teleBook.end())
	{
		std::cout << std::this_thread::get_id() << "\t" << na << ": " << teleBook[na] << std::endl;
	}
	else
	{
		std::cout << std::this_thread::get_id() << "\t" << na << " not found!" << std::endl;
	}
	std::this_thread::sleep_for(0.5s);
}

void testSharedMutex()
{
	std::thread reader8([] { printNumber("Bjarne"); });
	std::thread reader5([] { printNumber("Scott"); });
	std::thread reader1([] { printNumber("Scott"); });
	std::thread reader2([] { printNumber("Ritchie"); });
	std::thread w1([] { addToTeleBook("Scott", 1968); });
	std::thread reader3([] { printNumber("Dijkstra"); });
	std::thread reader4([] { printNumber("Scott"); });
	std::thread w2([] { addToTeleBook("Bjarne", 1965); });
	std::thread reader6([] { printNumber("Ritchie"); });
	std::thread reader7([] { printNumber("Scott"); });

	reader1.join();
	reader2.join();
	reader3.join();
	reader4.join();
	reader5.join();
	reader6.join();
	reader7.join();
	reader8.join();
	w1.join();
	w2.join();

	std::cout << "The new telephone book" << std::endl;
	for (auto teleIt : teleBook)
	{
		std::cout << teleIt.first << " : " << teleIt.second << std::endl;
	}
}