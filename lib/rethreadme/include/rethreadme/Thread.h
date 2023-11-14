#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <semaphore>
#include <thread>
#include <type_traits>

using namespace std::chrono_literals;

namespace rethreadme {

template <typename Function, typename... Args>
class Thread
{
public:
    Thread()
    {
        std::lock_guard lock(m_parameters->mtx);
        m_parameters->thread = std::thread([this]() { loop(m_parameters); });
    }

    Thread(const Function& function, Args&&... args)
        : Thread()
    {
        queue(function, std::forward<Args>(args)...);
    }

    Thread(Thread&& other) noexcept
    {
        if (this == &other)
        {
            return;
        }
        while (!other.isMoveable())
        {
            std::this_thread::sleep_for(1ms);
        }
        std::lock_guard lock(other.m_parameters->mtx);
        m_parameters = other.m_parameters;
        other.m_parameters = nullptr;
    }

    Thread& operator=(Thread&& other) noexcept
    {
        if (this == &other)
        {
            return *this;
        }
        this->~Thread();
        while (!other.isMoveable())
        {
            std::this_thread::sleep_for(1ms);
        }
        std::lock_guard lock(other.m_parameters->mtx);
        m_parameters = other.m_parameters;
        other.m_parameters = nullptr;
        return *this;
    }

    Thread(const Thread&) = delete;

    Thread& operator=(const Thread&) = delete;

    ~Thread()
    {
        if (!m_parameters)
        {
            return;
        }
        m_parameters->deinitSemaphore.acquire();
        {
            std::lock_guard lock(m_parameters->mtx);
            m_parameters->running = false;
        }
        m_parameters->thread.join();
        m_parameters = nullptr;
    }

    bool empty() const
    {
        std::lock_guard lock(m_parameters->mtx);
        return m_parameters->functions.empty() && !m_parameters->lastFunction.has_value();
    }

    operator bool() { return !empty(); }

    void queue(const Function& function, Args&&... args)
    {
        std::lock_guard lock(m_parameters->mtx);
        m_parameters->functions.push(function);
        m_parameters->argsRef.push(std::forward_as_tuple(std::forward<Args>(args)...));
        m_parameters->functionsSemaphore.release();
    }

    bool runLast()
    {
        {
            std::lock_guard lock(m_parameters->mtx);
            if (!m_parameters->lastFunction)
            {
                return false;
            }
        }
        queue(*(m_parameters->lastFunction.value()), *(m_parameters->lastFunctionArgs.value()));
        return true;
    }

private:
    struct Parameters
    {
        mutable std::mutex mtx{};
        std::counting_semaphore<> functionsSemaphore{0};
        std::binary_semaphore deinitSemaphore{0};
        std::once_flag deinitFlag{};
        std::thread thread;
        std::atomic<bool> running{true};
        std::atomic<bool> isMoveable{};

        std::queue<Function> functions;
        std::queue<std::tuple<Args...>> argsRef;

        std::optional<std::shared_ptr<Function>> lastFunction{std::nullopt};
        std::optional<std::shared_ptr<std::tuple<Args...>>> lastFunctionArgs{std::nullopt};
    };

    bool isMoveable() const
    {
        if (!m_parameters)
        {
            return false;
        }
        return m_parameters->isMoveable.load();
    }

    void loop(std::shared_ptr<Parameters> parameters)
    {
        parameters->isMoveable.store(true);
        while (parameters->running)
        {
            std::call_once(parameters->deinitFlag, [parameters]() { parameters->deinitSemaphore.release(); });
            if (!parameters->functionsSemaphore.try_acquire_for(1ms))
            {
                continue;
            }
            std::lock_guard lock(parameters->mtx);
            loopImpl(parameters);
        }
    }

    void queue(const Function& function, std::tuple<Args...> args)
    {
        std::lock_guard lock(m_parameters->mtx);
        m_parameters->functions.push(function);
        m_parameters->argsRef.push(args);
        m_parameters->functionsSemaphore.release();
    }

    void callerWithArgs(std::shared_ptr<Parameters> parameters)
    {
        auto function = std::make_shared<Function>(std::move(parameters->functions.front()));
        auto args = std::make_shared<std::tuple<Args...>>(std::move(parameters->argsRef.front()));
        parameters->functions.pop();
        parameters->argsRef.pop();
        parameters->lastFunction = function;
        parameters->lastFunctionArgs = args;
        std::apply(*(parameters->lastFunction.value()), (*(parameters->lastFunctionArgs.value())));
    }

    void callerNoArgs(std::shared_ptr<Parameters> parameters)
    {
        auto function = std::make_shared<Function>(std::move(parameters->functions->front()));
        parameters->functions->pop();
        parameters->lastFunction = std::move(function);
        (*function)();
    }

    void loopImpl(std::shared_ptr<Parameters> parameters)
    {
        if constexpr (std::is_invocable_v<Function, Args...> || std::is_invocable_v<Function, Args&...> ||
                      std::is_invocable_v<Function, Args&&...>)
        {
            callerWithArgs(parameters);
        }
        else
        {
            callerNoArgs(parameters);
        }
    }

private:
    // Pointer to allow easy move semantics
    std::shared_ptr<Parameters> m_parameters{std::make_shared<Parameters>()};
};

} // namespace rethreadme
