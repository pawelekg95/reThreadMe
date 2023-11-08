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

namespace rethreadme {

template <typename Function, typename... Args>
class Thread
{
public:
    Thread()
    {
        m_thread = std::thread([this]() { loop(); });
    }

    Thread(const Function& function, Args&&... args)
        : Thread()
    {
        queue(function, std::forward<Args>(args)...);
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

    void queue(const Function& function, Args&&... args)
    {
        std::lock_guard lock(m_mtx);
        m_functions.push(function);
        m_argsRef.push(std::forward_as_tuple(std::forward<Args>(args)...));
        m_functionsSemaphore.release();
    }

    bool runLast()
    {
        {
            std::lock_guard lock(m_mtx);
            if (!m_lastFunction)
            {
                return false;
            }
        }
        queue(*(m_lastFunction.value()), *(m_lastFunctionArgs.value()));
        return true;
    }

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
            loopImpl();
        }
    }

    void queue(const Function& function, std::tuple<Args...> args)
    {
        std::lock_guard lock(m_mtx);
        m_functions.push(function);
        m_argsRef.push(args);
        m_functionsSemaphore.release();
    }

    void callerWithArgs()
    {
        std::unique_ptr<Function> function{};
        std::unique_ptr<std::tuple<Args...>> args{};
        {
            std::lock_guard lock(m_mtx);
            function = std::make_unique<Function>(std::move(m_functions.front()));
            args = std::make_unique<std::tuple<Args...>>(std::move(m_argsRef.front()));
            m_functions.pop();
            m_argsRef.pop();
            m_lastFunction = std::move(function);
            m_lastFunctionArgs = std::move(args);
        }
        std::apply(*(m_lastFunction.value()), (*(m_lastFunctionArgs.value())));
    }

    void callerNoArgs()
    {
        std::unique_ptr<Function> function{};
        {
            std::lock_guard lock(m_mtx);
            function = std::make_unique<Function>(std::move(m_functions.front()));
            m_functions.pop();

            m_lastFunction = std::move(function);
        }
        (*function)();
    }

    void loopImpl()
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
    mutable std::mutex m_mtx{};
    std::counting_semaphore<> m_functionsSemaphore{0};
    std::binary_semaphore m_deinitSemaphore{0};
    std::once_flag m_deinitFlag;
    std::thread m_thread;
    std::atomic<bool> m_running{true};

    std::queue<Function> m_functions;
    std::queue<std::tuple<Args...>> m_argsRef;

    std::optional<std::unique_ptr<Function>> m_lastFunction{std::nullopt};
    std::optional<std::unique_ptr<std::tuple<Args...>>> m_lastFunctionArgs{std::nullopt};
};

} // namespace rethreadme
