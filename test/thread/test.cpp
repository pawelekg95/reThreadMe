#include "rethreadme/Thread.h"

#include <catch2/catch_all.hpp>

#include <functional>
#include <iostream>
#include <semaphore>

std::binary_semaphore semaphore{0}; // NOLINT

void printSome(int a)
{
    std::cout << "Print func: " << a << std::endl;
}

void modifySomeValue(int& a)
{
    a++;
    semaphore.release();
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
    std::this_thread::sleep_for(10ms);

    rethreadme::Thread thread3(&printSome, 1);
    thread3.queue(&printSome, 2);
    thread3.queue(&printSome, 3);
    thread3.queue(&printSome, 4);
    thread3.queue(&printSome, 5);
    std::this_thread::sleep_for(10ms);

    int someVal = 10;
    rethreadme::Thread<void (*)(int&), int&> thread4(&modifySomeValue, someVal);
    semaphore.acquire();
    REQUIRE(someVal == 11);

    thread4.queue(&modifySomeValue, someVal);
    semaphore.acquire();
    REQUIRE(someVal == 12);

    REQUIRE(thread4.runLast());
    semaphore.acquire();
    REQUIRE(someVal == 13);

    someVal = 1;
    REQUIRE(someVal == 1);
    REQUIRE(thread4.runLast());
    semaphore.acquire();
    REQUIRE(someVal == 2);

    rethreadme::Thread<void (*)(int&), int&> emptyThread;

    REQUIRE(thread1);
    REQUIRE(thread2);
    REQUIRE(thread3);
    REQUIRE(thread4);
    REQUIRE(!emptyThread);
    REQUIRE(emptyThread.empty());
    REQUIRE(!emptyThread.runLast());

    emptyThread.queue(&modifySomeValue, someVal);
    semaphore.acquire();
    REQUIRE(someVal == 3);

    REQUIRE(emptyThread);
    REQUIRE(!emptyThread.empty());
    REQUIRE(emptyThread.runLast());
    semaphore.acquire();
    REQUIRE(someVal == 4);
}
