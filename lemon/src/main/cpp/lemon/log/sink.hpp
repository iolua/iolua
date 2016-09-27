#ifndef LEMON_LOG_SINK_HPP
#define LEMON_LOG_SINK_HPP

#include <vector>
#include <string>
#include <unordered_set>
#include <lemon/nocopy.hpp>

namespace lemon{
	namespace log{

		enum class display_flag
		{
			timestamp = 1, source = 2, file_lines = 4, level = 8
		};

		/**
         * the predeclared of message structure
         */
		struct message;

		class sink
		{
		public:

			sink():_apply_all(true){}

			sink(const std::vector<std::string> & sources)
					:_apply_all(false),_apply_sources(sources.begin(),sources.end())
			{

			}

			virtual void write(const message & msg) = 0;
			virtual ~sink(){}

			bool apply_all() const
			{
				return _apply_all;
			}

			bool apply(const std::string & source) const
			{
				return _apply_all || _apply_sources.count(source) == 1;
			}

			void set_display_flags(std::uint32_t flags)
			{
				_display_flags = flags;
			}

			std::uint32_t get_display_flags() const
			{
				return _display_flags;
			}

			bool check_display_flag(display_flag flag)
			{
				return (_display_flags & (std::uint32_t)flag) == (std::uint32_t)flag;
			}

		private:

			bool								_apply_all;
			std::unordered_set<std::string>		_apply_sources;
			std::uint32_t						_display_flags;
		};

		/**
         * this is the console log sink implement
         */
		class console : public sink,private nocopy
		{
		public:
			console() = default;

			console(const std::vector<std::string> & sources)
					:sink(sources)
			{

			}

			void write(const message & msg) final;
		};
	}
}


#endif //LEMON_LOG_SINK_HPP