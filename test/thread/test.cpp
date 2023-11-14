#include "rethreadme/Thread.h"

#include <catch2/catch_all.hpp>

#include <functional>
#include <iostream>
#include <mutex>
#include <semaphore>

using namespace std::chrono_literals;

void printSome(int a, std::mutex& mtx)
{
    std::lock_guard lock(mtx);
    std::cout << "Print func: " << a << std::endl;
}

void prints(int a)
{
    std::cout << "Print func: " << a << std::endl;
}

void modifySomeValue(int& a, std::mutex& mtx, std::counting_semaphore<>& smph)
{
    std::lock_guard lock(mtx);
    a++;
    smph.release();
}

TEST_CASE("Creation")
{
    std::counting_semaphore<> semaphore{0};
    std::mutex mutex{};

    rethreadme::Thread thread1(std::function([&mutex]() {
        std::lock_guard lock(mutex);
        std::cout << "It works\n";
    }));

    thread1.runLast();

    rethreadme::Thread thread2(std::function([&mutex](int i) {
                                   std::lock_guard lock(mutex);
                                   std::cout << "This works as well: " << i << std::endl;
                               }),
                               122222);
    REQUIRE(true);
    thread1.queue(std::function([]() { std::cout << "Again, it works!\n"; }));
    thread2.queue(std::function<void(int)>(
                      [](int i) { std::cout << "This works as well but different arg: " << i << std::endl; }),
                  666);

    rethreadme::Thread threadWithLambda([]() { std::cout << "Lambda thread\n"; });

    rethreadme::Thread threadWithLambdaAndArguments(
        [](int a, int b) { std::cout << "Lambda thread and args\n"
                                     << a << " " << b << std::endl; },
        1,
        2);

    std::this_thread::sleep_for(100ms);
    REQUIRE(threadWithLambdaAndArguments.runLast());

    rethreadme::Thread thread3(&prints, 1);
    thread3.queue(&prints, 2);
    thread3.queue(&prints, 3);
    thread3.queue(&prints, 4);
    thread3.queue(&prints, 5);

    int someVal = 10;
    rethreadme::
        Thread<void (*)(int&, std::mutex&, std::counting_semaphore<>&), int&, std::mutex&, std::counting_semaphore<>&>
            thread4(&modifySomeValue, someVal, mutex, semaphore);
    semaphore.acquire();

    {
        std::lock_guard lock(mutex);
        REQUIRE(someVal == 11);
    }

    thread4.queue(&modifySomeValue, someVal, mutex, semaphore);
    semaphore.acquire();
    {
        std::lock_guard lock(mutex);
        REQUIRE(someVal == 12);
    }

    REQUIRE(thread4.runLast());
    semaphore.acquire();
    {
        std::lock_guard lock(mutex);
        REQUIRE(someVal == 13);
    }

    {
        std::lock_guard lock(mutex);
        someVal = 1;
        REQUIRE(someVal == 1);
    }
    REQUIRE(thread4.runLast());
    semaphore.acquire();
    {
        std::lock_guard lock(mutex);
        REQUIRE(someVal == 2);
    }

    rethreadme::
        Thread<void (*)(int&, std::mutex&, std::counting_semaphore<>&), int&, std::mutex&, std::counting_semaphore<>&>
            emptyThread;

    REQUIRE(thread1);
    REQUIRE(thread2);
    REQUIRE(thread3);
    REQUIRE(thread4);
    REQUIRE(!emptyThread);
    REQUIRE(emptyThread.empty());
    REQUIRE(!emptyThread.runLast());

    emptyThread.queue(&modifySomeValue, someVal, mutex, semaphore);
    semaphore.acquire();
    {
        std::lock_guard lock(mutex);
        REQUIRE(someVal == 3);
    }

    REQUIRE(emptyThread);
    REQUIRE(!emptyThread.empty());
    REQUIRE(emptyThread.runLast());
    semaphore.acquire();
    {
        std::lock_guard lock(mutex);
        REQUIRE(someVal == 4);
    }
}

TEST_CASE("Moving")
{
    rethreadme::Thread<std::function<void()>> thread1;
    REQUIRE(!thread1);
    REQUIRE(thread1.empty());

    thread1.queue(std::function([]() { std::cout << "It works\n"; }));

    REQUIRE(thread1);
    REQUIRE(!thread1.empty());

    rethreadme::Thread thread2{std::move(thread1)};
    thread2.queue(std::function([]() { std::cout << "It works, but from another thread\n"; }));

    thread2 = rethreadme::Thread<std::function<void()>>();
    thread2.queue(
        std::function([]() { std::cout << "It works, but from thread created with move assignment operator\n"; }));

    rethreadme::Thread<std::function<void()>> thread3;
    thread3 = std::move(thread2);
    thread3.queue(
        std::function([]() { std::cout << "It works, again in moved thread\n"; }));
}
