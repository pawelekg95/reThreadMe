#include "rethreadme/Thread.h"

#include <catch2/catch_all.hpp>

#include <functional>
#include <iostream>

void printSome(int a)
{
    std::cout << "Print func: " << a << std::endl;
}

void modifySomeValue(int& a)
{
    a++;
}

TEST_CASE("")
{
    using namespace std::chrono_literals;
    rethreadme::Thread thread1(std::function<void()>([]() { std::cout << "It works\n"; }));
    rethreadme::Thread thread2(
        std::function<void(int)>([](int i) { std::cout << "This works as well: " << i << std::endl; }),
        122222);
    REQUIRE(true);
    thread1.queue(std::function<void()>([]() { std::cout << "Again, it works!\n"; }));
    thread2.queue(std::function<void(int)>(
                      [](int i) { std::cout << "This works as well but different arg: " << i << std::endl; }),
                  666);
    std::this_thread::sleep_for(100ms);

    rethreadme::Thread thread3(&printSome, 1);
    thread3.queue(&printSome, 2);
    thread3.queue(&printSome, 3);
    thread3.queue(&printSome, 4);
    thread3.queue(&printSome, 5);
    std::this_thread::sleep_for(100ms);

    int someVal = 10;
    rethreadme::Thread thread4(&modifySomeValue, someVal);
}
