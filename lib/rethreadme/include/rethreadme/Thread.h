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

    mutable std::mutex m_mtx{};
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
            if (!m_functionsSemaphore.try_acquire_for(1ms))
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
    Thread() = default;

    Thread(const Function& function) { queue(function); }

    template <
        typename = std::enable_if_t<std::is_invocable_v<Function, Args...> || std::is_invocable_v<Function, Args&...> ||
                                    std::is_invocable_v<Function, Args&&...>>>
    Thread(const Function& function, Args&&... args)
    {
        queue(function, std::forward<Args&&...>(args)...);
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

    bool empty() const
    {
        std::lock_guard lock(m_mtx);
        return m_functions.empty() && !m_lastFunction.has_value();
    }

    operator bool() { return !empty(); }

    template <
        typename = std::enable_if_t<std::is_invocable_v<Function, Args...> || std::is_invocable_v<Function, Args&...> ||
                                    std::is_invocable_v<Function, Args&&...>>>
    void queue(const Function& function, Args&&... args)
    {
        std::lock_guard lock(m_mtx);
        m_functions.push(function);
        m_argsRef.push(std::forward<Args&&...>(args)...);
        m_functionsSemaphore.release();
    }

    void queue(const Function& function)
    {
        std::lock_guard lock(m_mtx);
        m_functions.push(function);
        m_functionsSemaphore.release();
    }

    bool runLast()
    {
        if (!m_lastFunction)
        {
            return false;
        }
        if (m_lastFunctionArgs)
        {
            queue(*m_lastFunction, *m_lastFunctionArgs);
        }
        else
        {
            queue(*m_lastFunction);
        }
        return true;
    }

private:
    void queue(const Function& function, std::tuple<Args&&...> args)
    {
        std::lock_guard lock(m_mtx);
        m_functions.push(function);
        m_argsRef.push(args);
        m_functionsSemaphore.release();
    }

    void callerWithArgs()
    {
        Function function{};
        std::unique_ptr<std::tuple<Args&&...>> args;
        {
            std::lock_guard lock(m_mtx);
            function = m_functions.front();
            args = std::make_unique<std::tuple<Args&&...>>(std::move(m_argsRef.front()));
            m_functions.pop();
            m_argsRef.pop();
            m_lastFunction = function;
            m_lastFunctionArgs = std::tuple<Args&&...>(std::move(*args));
        }
        std::apply(function, (*m_lastFunctionArgs));
    }

    void callerNoArgs()
    {
        Function function{};
        {
            std::lock_guard lock(m_mtx);
            function = m_functions.front();
            m_functions.pop();

            m_lastFunction = function;
        }
        function();
    }

    void loopImpl() override
    {
        if constexpr (std::is_invocable_v<Function, Args...> || std::is_invocable_v<Function, Args&...> ||
                      std::is_invocable_v<Function, Args&&...>)
        {
            callerWithArgs();
        }
        else
        {
            callerNoArgs();
        }
    }

private:
    std::queue<Function> m_functions;
    std::queue<std::tuple<Args&&...>> m_argsRef;

    std::optional<Function> m_lastFunction{std::nullopt};
    std::optional<std::tuple<Args&&...>> m_lastFunctionArgs{std::nullopt};
};

} // namespace rethreadme
