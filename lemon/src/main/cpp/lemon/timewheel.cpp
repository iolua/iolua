#include <lemon/timewheel.hpp>
#include <cassert>

namespace lemon {
    timer_wheel::timer_wheel(size_t ticksOfMillisecond) : _exit(false),_tickOfMilliseconds(ticksOfMillisecond)
    {

    }

    timer_wheel::~timer_wheel()
    {
        if(_worker.joinable())
        {
            {
                std::unique_lock<std::mutex> lock(_mutex);

                _exit = true;
            }


            _worker.join();
        }
    }


    void timer_wheel::stop_and_join()
    {
        {
            std::unique_lock<std::mutex> lock(_mutex);

            _exit = true;

            _condition.notify_one();
        }

        if (_worker.joinable()) _worker.join();
    }


    void timer_wheel::start()
    {
        _worker = std::thread(&timer_wheel::proc, this);
    }

    void timer_wheel::create_timer(std::function<void()> callback, std::chrono::milliseconds timeout)
    {
        std::unique_lock<std::mutex> lock(_mutex);

        timer * t = new_timer(callback, timeout);

        insert(t);
    }

    timer_wheel::timer * timer_wheel::new_timer(std::function<void()> callback, std::chrono::milliseconds timeout)
    {
        size_t counter = (size_t) (timeout.count() / _tickOfMilliseconds + 1);

        return new timer(callback, counter );
    }

    void timer_wheel::free_timer(timer_wheel::timer * val)
    {
        delete val;
    }

    void timer_wheel::insert(timer_wheel::timer * t)
    {
        size_t buckets;

        cascade * cas;

        std::uint32_t tick = t->ticks;

        assert(t->next == NULL && t->prev == NULL);

        if (0 == tick)
        {
            _mutex.unlock();

            t->target();

            _mutex.lock();

            free_timer(t);

            _timers--;

            return;
        }

        if ((tick >> 24) & 0xff)
        {
            buckets = ((tick >> 24) & 0xff);

            cas = &_cascades[3];

        }
        else if ((tick >> 16) & 0xff)
        {
            buckets = ((tick >> 16) & 0xff);

            cas = &_cascades[2];
        }
        else if ((tick >> 8) & 0xff)
        {
            buckets = ((tick >> 8) & 0xff);

            cas = &_cascades[1];
        }
        else
        {
            buckets = tick;

            cas = &_cascades[0];
        }

        buckets = (buckets + cas->cursor - 1) % 256;

        t->next = cas->buckets[buckets];

        t->prev = &cas->buckets[buckets];

        if (cas->buckets[buckets]){

            assert(cas->buckets[buckets]->prev == &cas->buckets[buckets]);

            cas->buckets[buckets]->prev = &t->next;
        }

        cas->buckets[buckets] = t;
    }

    void timer_wheel::__cascade(size_t index)
    {
        cascade *cas = &_cascades[index];

        if (cas->cursor != 0 || index == 3) return;

        cascade *cascade_upper = &_cascades[++index];

        timer *timers = cascade_upper->buckets[cascade_upper->cursor];

        cascade_upper->buckets[cascade_upper->cursor++] = NULL;

        __cascade(index);

        while (timers){

            timer *next = timers->next;


            switch (index)
            {
                case 1:
                {
                    timers->ticks &= 0xff;

                    break;
                }
                case 2:
                {
                    timers->ticks &= 0xffff;

                    break;
                }
                case 3:
                {
                    timers->ticks &= 0xffffff;

                    break;
                }
                default:
                {
                    assert(false);
                }
            }

            timers->next = NULL; timers->prev = NULL;

            insert(timers);

            timers = next;

        };
    }

    void timer_wheel::tick()
    {
        cascade *cas = &_cascades[0];

        timer * timers = cas->buckets[cas->cursor];

        cas->buckets[cas->cursor] = NULL;

        cas->cursor++;

        __cascade(0);

        while (timers){

            _timers--;

            timer * next = timers->next;

            timers->next = NULL; timers->prev = NULL;

            _mutex.unlock();

            timers->target();

            _mutex.lock();

            free_timer(timers);

            timers = next;
        }
    }

    void timer_wheel::proc()
    {
        std::unique_lock<std::mutex> lock(_mutex);

        while (!_exit)
        {
            if (std::cv_status::timeout == _condition.wait_for(lock, std::chrono::milliseconds(_tickOfMilliseconds)))
            {
                tick();
            }
        }
    }
}