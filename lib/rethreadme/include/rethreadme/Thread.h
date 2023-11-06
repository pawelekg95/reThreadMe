#pragma once

#include <atomic>
#include <chrono>
#include <list>
#include <mutex>
#include <optional>
#include <queue>
#include <semaphore>
#include <thread>
#include <type_traits>

namespace {

class ThreadImpl
{
public:
    ThreadImpl()
    {
        m_thread = std::thread([this]() { loop(); });
    }

    virtual ~ThreadImpl(){};

    virtual void loopImpl() = 0;

    std::mutex m_mtx{};
    std::counting_semaphore<> m_functionsSemaphore{0};
    std::binary_semaphore m_deinitSemaphore{0};
    std::thread m_thread;
    std::atomic<bool> m_running{true};

private:
    void loop()
    {
        using namespace std::chrono_literals;
        while (m_running)
        {
            std::call_once(m_deinitFlag, [this]() { m_deinitSemaphore.release(); });
            if (!m_functionsSemaphore.try_acquire_for(10ms))
            {
                continue;
            }
            this->loopImpl();
        }
    }

    std::once_flag m_deinitFlag{};
};

} // namespace

namespace rethreadme {

template <typename Function, typename... Args>
class Thread : public ThreadImpl
{
public:
    Thread(const Function& function) { queue(function); }

    template <typename = std::enable_if_t<std::is_invocable_v<Function, Args&...>>>
    Thread(const Function& function, Args& ...args)
    {
        queue(function, std::forward<Args&...>(args)...);
    }


    ~Thread()
    {
        m_deinitSemaphore.acquire();
        {
            std::lock_guard lock(m_mtx);
            m_running = false;
        }
        m_thread.join();
    }

    template <typename = std::enable_if_t<std::is_invocable_v<Function, Args&...>>>
    void queue(const Function& function, Args&... args)
    {
        std::lock_guard lock(m_mtx);
        m_functions.push(function);
        m_argsRef.push(std::forward<Args&...>(args)...);
        m_functionsSemaphore.release();
    }

    void queue(const Function& function)
    {
        std::lock_guard lock(m_mtx);
        m_functions.push(function);
        m_functionsSemaphore.release();
    }

private:
    void callerWithArgsRef()
    {
        Function function{};
        std::tuple<Args&...> args;
        {
            std::lock_guard lock(m_mtx);
            function = m_functions.front();
            args = m_argsRef.front();
            m_functions.pop();
            m_argsRef.pop();
        }
        std::apply(function, args);
    }

    void callerNoArgs()
    {
        Function function{};
        {
            std::lock_guard lock(m_mtx);
            function = m_functions.front();
            m_functions.pop();
        }
        function();
    }

    void loopImpl() override
    {
        if constexpr (std::is_invocable<Function, Args&...>::value)
        {
            callerWithArgsRef();
        }
        else
        {
            callerNoArgs();
        }
    }

private:
    std::queue<Function> m_functions;
    std::queue<std::tuple<Args&...>> m_argsRef;
};

} // namespace rethreadme
