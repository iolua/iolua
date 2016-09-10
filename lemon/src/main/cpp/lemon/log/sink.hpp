#ifndef LEMON_LOG_SINK_HPP
#define LEMON_LOG_SINK_HPP

#include <vector>
#include <string>
#include <unordered_set>
#include <lemon/nocopy.hpp>

namespace lemon{ namespace log{

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

	private:

		bool								_apply_all;
		std::unordered_set<std::string>		_apply_sources;
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
}}


#endif //LEMON_LOG_SINK_HPP