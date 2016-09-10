#include <chrono>
#include <iostream>
#include <lemon/log/log.hpp>
#include <lemon/test/unit.hpp>
#include <lemon/test/runner.hpp>

namespace lemon{ namespace test{

    unit::unit(const std::string & name, const std::string & filename, int lines)
        :_name(name),_filename(filename),_lines(lines)
    {
        runner::instance().add(this);
    }

    const std::string&unit::name() const
    {
        return _name;
    }

    int unit::lines() const
    {
        return _lines;
    }

    const std::string&unit::file() const
    {
        return _filename;
    }

    T::T(const std::string & name, const std::string & filename, int lines)
        :unit(name,filename,lines)
    {

    }

    void T::run()
    {
        main();
    }

    B::B(const std::string &name, const std::string &filename, int lines)
        :unit(name,filename,lines)
    {

    }

	void B::stop_timer()
	{
		_stop_timepoint = std::chrono::high_resolution_clock::now();
	}

	void B::start_timer()
	{
		_stopduration += 
			std::chrono::duration_cast<std::chrono::nanoseconds>(
			std::chrono::high_resolution_clock::now() - _stop_timepoint
			);
	}

    void B::run()
    {
        // first test

        N = 1;

        using clock = std::chrono::high_resolution_clock;

        namespace chrono = std::chrono;

        auto start  = clock::now();

        main();

        auto duration = chrono::duration_cast<chrono::nanoseconds>(clock::now() - start);

		duration -= _stopduration;

		_stopduration = chrono::nanoseconds::zero();

        if (duration > chrono::seconds(1))
        {
            return;
        }

		if(duration.count() == 0)
		{
			N = 100000;
		}
		else
		{
			N = (int)(chrono::duration_cast<chrono::nanoseconds>(chrono::seconds(1)).count() / duration.count());
		}

        start  = clock::now();

        main();

        duration = chrono::duration_cast<chrono::nanoseconds>(clock::now() - start) - _stopduration;

		lemonI(log::get("test") , "benchmark(%s)\t%d ns/op", name().c_str(),duration.count() / N);
    }
}}