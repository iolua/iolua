#ifndef IOLUA_SHARED_OBJECT_HPP
#define IOLUA_SHARED_OBJECT_HPP

#include <atomic>
#include <cstdint>
#include <type_traits>
#include <forward_list>
#include <unordered_map>
#include <lemon/nocopy.hpp>
#include <lemon/mutex.hpp>

namespace iolua {

    class shared_object : private lemon::nocopy
    {
	public:

		virtual ~shared_object() {}

		void addref()
		{
			++_counter;
		}
		void unref()
		{
			if( -- _counter  == 0) 
			{
				close_object();
			}
		}

		void set_id(std::uint32_t id)
		{
			_id = id;
		}

		std::uint32_t id() const
		{
			return _id;
		}

	protected:
		shared_object() {}
    private:
        void close_object() 
		{
			delete this;
		}

	private:
		std::atomic_int			_counter = {1};
		std::uint32_t			_id;
    };

	template<typename _Type>
	class shared_object_cached
	{
		static_assert(std::is_base_of<shared_object, _Type>::value, "_T must be a sub class of shared_object ");
	public:

		std::uint32_t attach(_Type * obj)
		{
			std::uint32_t id;

			std::unique_lock<lemon::spin_mutex> lock(_mutex);

			for (;;)
			{
				id = _idgen++;

				if (id != 0 && _cached.count(id) == 0) break;
			}

			obj->set_id(id);

			_cached[id] = obj;

			return id;
		}

		template<typename ... Args>
		std::uint32_t create(Args && ...args)
		{
			std::uint32_t id;

			std::unique_lock<lemon::spin_mutex> lock(_mutex);

			for (;;)
			{
				id = _idgen++;

				if (id != 0 && _cached.count(id) == 0) break;
			}


			auto obj = new _Type(std::forward<Args>(args)...);

			obj->set_id(id);

			_cached[id] = obj;

			return id;
		}

		void close(std::uint32_t id)
		{
			std::unique_lock<lemon::spin_mutex> lock(_mutex);

			auto iter = _cached.find(id);

			if (iter != _cached.end())
			{
				iter->second->unref();

				_cached.erase(iter);
			}
		}

		_Type* addref_and_fetch(std::uint32_t id)
		{
			std::unique_lock<lemon::spin_mutex> lock(_mutex);

			auto iter = _cached.find(id);

			if (iter != _cached.end())
			{
				iter->second->addref();

				return iter->second;
			}

			return nullptr;
		}


	private:
		std::uint32_t								_idgen = 0;
		lemon::spin_mutex							_mutex;
		std::unordered_map<std::uint32_t, _Type*>	_cached;
	};
}

#endif //IOLUA_SHARED_OBJECT_HPP