// test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <string>
#include "../deadlock.h"

int main()
{
	while (true)
	{
		std::string s;
		std::cout << "\
		0: deadlock\n\
		1: fix deadlock by std::lock and std::lock_guard\n\
		2: fix deadlock by std::lock and std::unique_lock\n\
		3: fix deadlock by std::scoped_lock\n\
		q: quit\n\
		choose: ";
		std::cin >> s;
		std::cout << std::endl;

		if (s == "q" || s == "Q")
		{
			break;
		}

		testDeadlock(std::stoi(s));
	}

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
