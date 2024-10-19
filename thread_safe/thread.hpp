#pragma once
#include "common/common.hpp"

#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <thread>

namespace ThreadSafe
{

/**
 * @brief Enum to represent thread priorities.
 */
enum class ThreadPriority : uint32_t
{
    LOWEST = 0,
    BELOW_NORMAL = 1,
    NORMAL = 2,
    ABOVE_NORMAL = 3,
    HIGHEST = 4,
    TIME_CRITICAL = 5
};

using NativeThreadPrioritys = std::map<ThreadPriority, int32_t>;

/**
 * @brief Provides the default mapping of ThreadPriority to native priority values.
 * @return The map of ThreadPriority to int32_t native priority values.
 */
const NativeThreadPrioritys& defaultNativeThreadPrioritys();

/**
 * @brief Sets the native thread priority for a given thread.
 * @param priority The desired thread priority.
 * @param native_handle The native handle of the thread to set priority.
 */
void setNaitiveThreadPriority(ThreadPriority priority, const std::thread::native_handle_type native_handle);

/**
 * @brief Enum to represent whether the thread should run once or in a loop.
 */
enum class RunMode : uint8_t
{
    ONCE = 0,
    LOOP = 1
};

/**
 * @brief A thread class that supports custom functions, thread priorities, and callbacks.
 * @tparam Return The return type of the function.
 * @tparam ArgTypes The types of the arguments for the function.
 */
template<typename Return, typename... ArgTypes>
class Thread
{
public:
    using Callback = std::function<void()>;
    using ResultCallback = std::function<void(const Return&)>;
    using Func = std::function<Return(ArgTypes...)>;
    using Pred = std::function<bool()>;

    /**
     * @brief Constructor for creating a thread with a function and its arguments.
     * @param name The name of the thread.
     * @param priority The priority of the thread.
     */
    template<class... Args>
    Thread(const std::string& name, const ThreadPriority priority = ThreadPriority::NORMAL)
        : m_name{name}
        , m_priority{priority}
    {
    }

    /**
     * @brief Destructor that ensures the thread stops when the object is destroyed.
     */
    ~Thread()
    {
        stop();
    }

    // Make this class uncopyable
    UNCOPYABLE(Thread);

    /**
     * @brief Sets the function and its arguments before starting the thread.
     * This function stores the function and arguments to be invoked when the thread is run.
     * @tparam Args The types of arguments for the function.
     * @param func The function to be executed.
     * @param args The arguments to be passed to the function.
     * @return `true` if the function and arguments were successfully set, `false` otherwise.
     */
    template<class... Args>
    bool invoke(Func func, Args&&... args)
    {
        if (m_thread_ptr != nullptr)
        {
            return false;
        }
        try
        {
            m_func = func;
            m_args = std::tuple<ArgTypes...>(std::forward<Args>(args)...);
        }
        catch (std::exception e)
        {
            LOG_ERROR("Failed to invoke functor: " << e.what());
            return false;
        }
        return true;
    }

    // Setters for member variables
    /**
     * @brief Sets the predicate to control the loop condition of the thread.
     * @param pred The predicate function that returns a boolean indicating whether the loop should continue.
     */
    void setPredicate(Pred pred)
    {
        m_pred = pred;
    }

    /**
     * @brief Sets the start callback function to be executed when the thread starts.
     * @param start_callback The callback function.
     */
    void setStartCallback(Callback start_callback)
    {
        m_start_callback = start_callback;
    }

    /**
     * @brief Sets the result callback function to be executed with the result of the function.
     * @param result_callback The callback function to handle the result.
     */
    void setResultCallback(ResultCallback result_callback)
    {
        m_result_callback = result_callback;
    }

    /**
     * @brief Sets the exit callback function to be executed when the thread exits.
     * @param exit_callback The callback function.
     */
    void setExitCallback(Callback exit_callback)
    {
        m_exit_callback = exit_callback;
    }

    /**
     * @brief Starts the thread.
     * @param loop Whether the thread should run once or in a loop.
     * @return `true` if start sucessfull, `false` otherwise.
     */
    bool start(const RunMode mode)
    {
        if (m_thread_ptr != nullptr)
        {
            LOG_WARNING("The thread has already started!")
            return false;
        }
        if (!m_func)
        {
            LOG_WARNING("Cannot start because the functor has not been invoked.")
            return false;
        }
        m_loop = false;
        if (mode == RunMode::LOOP)
        {
            m_loop = true;
        }
        m_thread_ptr = std::make_unique<std::thread>([this]()
                                                     { run(); });
        LOG_INFO("Successfully started the thread");
        return true;
    }
    /**
     * @brief Stops the thread's loop.
     * @return `true` if stop sucessfull, `false` otherwise.
     */
    bool stop()
    {
        m_loop = false;
        if (!m_thread_ptr)
        {
            LOG_WARNING("The thread has already stopped!")
            return false;
        }
        if (m_thread_ptr->joinable())
        {
            m_thread_ptr->join();
        }
        m_thread_ptr.reset();
        LOG_INFO("Successfully stopped the thread");
        return true;
    }

    /**
     * @brief Returns the name of the thread.
     * @return The name of the thread.
     */
    std::string name() const
    {
        return m_name;
    }

private:
    const std::string m_name;
    Func m_func{nullptr};
    std::tuple<ArgTypes...> m_args{};
    std::atomic<bool> m_loop{true};
    ThreadPriority m_priority;
    Pred m_pred{};
    Callback m_start_callback{};
    ResultCallback m_result_callback{};
    Callback m_exit_callback{};
    std::unique_ptr<std::thread> m_thread_ptr{};

    /**
     * @brief The main loop function that runs the thread.
     */
    void run()
    {
        setNaitiveThreadPriority(m_priority, m_thread_ptr->native_handle());
        startCallback();

        do
        {
            call();
        } while (isContinue());

        exitCallback();
    }

    /**
     * @brief Function to execute the function stored in the thread.
     */
    void call()
    {
        Return result{std::apply(m_func, m_args)};
        if (m_result_callback)
        {
            m_result_callback(result);
        };
    }

    /**
     * @brief Function to execute the start callback.
     */
    void startCallback()
    {
        if (m_start_callback)
        {
            m_start_callback();
        };
    }

    /**
     * @brief Function to execute the exit callback.
     */
    void exitCallback()
    {
        if (m_exit_callback)
        {
            m_exit_callback();
        };
    }

    /**
     * @brief Function to determine whether the thread should continue running.
     * @return True if the thread should continue running, false otherwise.
     */
    bool isContinue()
    {
        if (!m_loop)
        {
            return false;
        }
        if (m_pred && !m_pred())
        {
            return false;
        }
        return true;
    }
};
} // namespace ThreadSafe