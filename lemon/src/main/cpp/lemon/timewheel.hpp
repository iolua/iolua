#ifndef LEMON_TIMEWHEEL_HPP
#define LEMON_TIMEWHEEL_HPP

#include <mutex>
#include <cstdint>
#include <chrono>
#include <thread>
#include <algorithm>
#include <functional>
#include <condition_variable>
#include <lemon/nocopy.hpp>
namespace lemon {

    class timer_wheel : nocopy
    {
    public:
        static uintptr_t event()
        {
            static const uintptr_t unknown = 0;

            return (uintptr_t)&unknown;
        }
        struct timer : nocopy
        {
            timer(std::function<void()> callback, size_t tk)
                    :next(nullptr)
                    , prev(nullptr)
                    , target(callback)
                    , ticks((std::uint32_t)tk)
            {

            }

            timer									*next;

            timer									**prev;

            std::function<void()>				    target;

            std::uint32_t							ticks;
        };

        struct cascade
        {
            cascade()
            {
                std::fill(std::begin(buckets), std::end(buckets), nullptr);
            }

            std::uint8_t							cursor;

            timer									*buckets[256];
        };

        timer_wheel(size_t tickOfMilliseconds);

        ~timer_wheel();

    public:

        void stop_and_join();

        void start();

        void create_timer(std::function<void()> callback, std::chrono::milliseconds timeout);

    private:

        timer * new_timer(std::function<void()> callback, std::chrono::milliseconds timeout);

        void free_timer(timer * val);

        void proc();

        void insert(timer * val);

        void tick();

        void __cascade(size_t index);

    private:

        bool										_exit;

        size_t										_timers;

        cascade										_cascades[4];

        std::thread									_worker;

        std::mutex									_mutex;

        std::condition_variable						_condition;

        size_t                                      _tickOfMilliseconds;
    };
}

#endif //