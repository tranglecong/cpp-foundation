#pragma once

#include "common/common.hpp"

#include "wait.hpp"

#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <limits>
#include <mutex>
#include <utility>

namespace ThreadSafe
{

/**
 * @brief Thread-safe queue class with discard and control policies.
 *
 * The Queue class allows thread-safe push and pop operations with optional control and discard policies.
 *
 * @tparam T Type of elements stored in the queue.
 */
template<typename T>
class Queue
{
public:
    using DiscardedCallback = std::function<void(const T&)>;
    static constexpr uint32_t WAIT_FOREVER = std::numeric_limits<uint32_t>::max();

    /**
     * @brief Status of the queue.
     */
    enum class Status
    {
        EMPTY = 0,  ///< The queue is empty.
        NORMAL = 1, ///< The queue is neither empty nor full.
        FULL = 2    ///< The queue is full.
    };

    /**
     * @brief Discard policy for the queue.
     */
    enum class Discard : uint32_t
    {
        DISCARD_OLDEST = 0, ///< Discard the oldest element when full.
        DISCARD_NEWEST = 1, ///< Discard the newest element when full.
        NO_DISCARD = 2      ///< Do not discard any elements.
    };

    /**
     * @brief Control policy for the queue operations.
     */
    enum class Control : uint32_t
    {
        PUSH = 1,         ///< Only push control.
        POP = 2,          ///< Only pop control.
        FULL_CONTROL = 3, ///< Full control for both push and pop.
        NO_CONTROL = 4    ///< No control for push and pop operations.
    };

    /**
     * @brief Settings for the queue, such as discard policy, control, and size.
     */
    struct Settings
    {
        Discard discard{Discard::NO_DISCARD};                 ///< Discard policy.
        Control control{Control::NO_CONTROL};                 ///< Control policy.
        std::size_t size{std::numeric_limits<size_t>::max()}; ///< Maximum size of the queue.
    };

    /**
     * @brief Constructor that accepts queue settings.
     * @param settings Settings to configure the queue behavior.
     */
    explicit Queue(const Settings& settings);

    // Make this class uncopyable
    UNCOPYABLE(Queue);

    /**
     * @brief Set the callback for discarded elements.
     * @param discarded_callback Function to be called when an element is discarded.
     */
    void setDiscardedCallback(DiscardedCallback discarded_callback);

    /**
     * @brief Open the queue for push operations.
     */
    void openPush();

    /**
     * @brief Close the queue for push operations.
     */
    void closePush();

    /**
     * @brief Open the queue for pop operations.
     */
    void openPop();

    /**
     * @brief Close the queue for pop operations.
     */
    void closePop();

    /**
     * @brief Push an element into the queue.
     * @param elem Element to push.
     * @param timeout_ms Timeout in milliseconds. Defaults to WAIT_FOREVER.
     * @return True if the element was pushed successfully, false otherwise.
     */
    bool push(const T& elem, const uint32_t timeout_ms = WAIT_FOREVER);

    /**
     * @brief Pop an element from the queue.
     * @param elem Reference to store the popped element.
     * @param timeout_ms Timeout in milliseconds. Defaults to WAIT_FOREVER.
     * @return True if an element was popped successfully, false otherwise.
     */
    bool pop(T& elem, const uint32_t timeout_ms = WAIT_FOREVER);

private:
    const Settings m_settings;                   ///< Queue settings.
    std::deque<T> m_queue{};                     ///< Underlying queue storage.
    std::atomic<std::size_t> m_size{0};          ///< Current size of the queue.
    std::atomic<Status> m_status{Status::EMPTY}; ///< Status of the queue.
    std::mutex m_lock{};                         ///< Mutex to protect the queue operations.
    std::atomic<bool> m_open_push{false};        ///< Flag indicating whether push is open.
    std::atomic<bool> m_open_pop{false};         ///< Flag indicating whether pop is open.
    Wait m_wait{};                               ///< Wait mechanism for blocking operations.
    DiscardedCallback m_discarded_callback{};    ///< Callback for discarded elements.

    void onDiscarded(const T& elem);            ///< Handle discarded elements.
    bool pushControllable() const;              ///< Check if push is controllable.
    bool popControllable() const;               ///< Check if pop is controllable.
    bool waitToPush(const uint32_t timeout_ms); ///< Wait for push availability.
    bool waitToPop(const uint32_t timeout_ms);  ///< Wait for pop availability.
    void pushWithLock(const T& elem);           ///< Internal push method.
    const T& popWithLock();                     ///< Internal pop method.
    void updateStatus();                        ///< Update the status of the queue.
};

template<typename T>
Queue<T>::Queue(const Settings& settings)
    : m_settings{settings}
{
    if (!pushControllable())
    {
        m_open_push = true;
    }
    if (!popControllable())
    {
        m_open_pop = true;
    }
}

template<typename T>
void Queue<T>::setDiscardedCallback(DiscardedCallback discarded_callback)
{
    m_discarded_callback = discarded_callback;
}

template<typename T>
void Queue<T>::onDiscarded(const T& elem)
{
    if (m_discarded_callback)
    {
        m_discarded_callback(elem);
    }
}

template<typename T>
bool Queue<T>::push(const T& elem, const uint32_t timeout_ms)
{
    if (!waitToPush(timeout_ms))
    {
        return false;
    }

    if (m_status != Status::FULL)
    {
        pushWithLock(elem);
        return true;
    }

    if (m_settings.discard == Discard::DISCARD_NEWEST)
    {
        onDiscarded(elem);
        return false;
    }

    if (m_settings.discard == Discard::DISCARD_OLDEST)
    {
        auto discarded_elem = std::move(popWithLock());
        onDiscarded(discarded_elem);
        return true;
    }
    return false;
}

template<typename T>
bool Queue<T>::pop(T& elem, const uint32_t timeout_ms)
{
    if (!waitToPop(timeout_ms))
    {
        return false;
    }
    elem = popWithLock();
    return true;
}

template<typename T>
bool Queue<T>::pushControllable() const
{
    if (m_settings.control == Control::FULL_CONTROL || m_settings.control == Control::PUSH)
    {
        return true;
    }
    return false;
}

template<typename T>
bool Queue<T>::popControllable() const
{
    if (m_settings.control == Control::FULL_CONTROL || m_settings.control == Control::POP)
    {
        return true;
    }
    return false;
}

template<typename T>
void Queue<T>::openPush()
{
    if (!pushControllable())
    {
        return;
    }
    m_open_push = true;
    m_wait.notify();
}

template<typename T>
void Queue<T>::closePush()
{
    if (!pushControllable())
    {
        return;
    }
    m_open_push = false;
    m_wait.notify();
}

template<typename T>
void Queue<T>::openPop()
{
    if (!popControllable())
    {
        return;
    }
    m_open_pop = true;
    m_wait.notify();
}

template<typename T>
void Queue<T>::closePop()
{
    if (!popControllable())
    {
        return;
    }
    m_open_pop = false;
    m_wait.notify();
}

template<typename T>
bool Queue<T>::waitToPush(const uint32_t timeout_ms)
{
    if (!m_open_push)
    {
        return false;
    }

    static auto closed_or_not_full_pred = [this]() -> bool
    {
        if (!m_open_push || m_status != Status::FULL)
        {
            return true;
        }
        return false;
    };

    if (m_status == Status::FULL && m_settings.discard == Discard::NO_DISCARD)
    {
        Wait::Status result{m_wait.waitFor(std::chrono::milliseconds(timeout_ms), closed_or_not_full_pred)};
        if (result != Wait::Status::SUCCESS || !m_open_push)
        {
            return false;
        }
    }
    return true;
}
template<typename T>
bool Queue<T>::waitToPop(const uint32_t timeout_ms)
{
    if (!m_open_pop)
    {
        return false;
    }

    static auto closed_or_not_empty_pred = [this]() -> bool
    {
        if (!m_open_push || m_status != Status::EMPTY)
        {
            return true;
        }
        return false;
    };

    if (m_status == Status::EMPTY)
    {
        Wait::Status result{m_wait.waitFor(std::chrono::milliseconds(timeout_ms), closed_or_not_empty_pred)};
        if (result != Wait::Status::SUCCESS || !m_open_pop)
        {
            return false;
        }
    }
    return true;
}

template<typename T>
void Queue<T>::pushWithLock(const T& elem)
{
    std::lock_guard<std::mutex> lock{m_lock};
    m_queue.push_back(elem);
    updateStatus();
}

template<typename T>
const T& Queue<T>::popWithLock()
{
    std::lock_guard<std::mutex> lock{m_lock};
    const T& elem{m_queue.front()};
    m_queue.pop_front();
    updateStatus();
    return std::move(elem);
}

template<typename T>
void Queue<T>::updateStatus()
{
    constexpr std::size_t NO_ELEMENT{0};
    m_size = m_queue.size();
    if (m_size <= NO_ELEMENT)
    {
        m_status = Status::EMPTY;
    }
    else if (m_size >= m_settings.size)
    {
        m_status = Status::FULL;
    }
    else
    {
        m_status = Status::NORMAL;
    }
    m_wait.notify();
}

} // namespace ThreadSafe