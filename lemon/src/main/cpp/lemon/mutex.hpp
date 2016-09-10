#ifndef LEMON_SHARED_MUTEX_HPP
#define LEMON_SHARED_MUTEX_HPP

#include <mutex>
#ifndef WIN32

#include <pthread.h>

namespace lemon {
    class shared_mutex
    {
    public:

        typedef pthread_rwlock_t native_handle_type;

        shared_mutex( const shared_mutex& ) = delete;

        shared_mutex()
        {
            _handler = PTHREAD_RWLOCK_INITIALIZER;
        }

        ~shared_mutex()
        {
            pthread_rwlock_destroy(&_handler);
        }

        void lock()
        {
            pthread_rwlock_wrlock(&_handler);
        }

        bool try_lock()
        {
            return  0 == pthread_rwlock_trywrlock(&_handler);
        }

        void unlock()
        {
            pthread_rwlock_unlock(&_handler);
        }

        void lock_shared()
        {
            pthread_rwlock_rdlock(&_handler);
        }

        bool try_lock_shared()
        {
            return  0 == pthread_rwlock_tryrdlock(&_handler);
        }

        void unlock_shared()
        {
            pthread_rwlock_unlock(&_handler);
        }

        native_handle_type native_handle()
        {
            return _handler;
        }

    private:
        pthread_rwlock_t    _handler;
    };
}
#else
#include <shared_mutex>

namespace lemon {
	using shared_mutex = std::shared_mutex;
}

#endif

#include <atomic>

namespace lemon {
    class spin_mutex
    {
    public:

        typedef std::atomic_flag* native_handle_type;

        spin_mutex( const shared_mutex& ) = delete;

        spin_mutex()
        {

        }

        ~spin_mutex()
        {

        }

        void lock()
        {
            while (_handler.test_and_set(std::memory_order_acquire));
        }

        bool try_lock()
        {
            return !_handler.test_and_set(std::memory_order_acquire);

        }

        void unlock()
        {
            _handler.clear(std::memory_order_release);
        }

        native_handle_type native_handle()
        {
            return &_handler;
        }

    private:
        std::atomic_flag _handler = ATOMIC_FLAG_INIT;
    };
}

#endif //