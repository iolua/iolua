#include <iostream>


#include <lemon/log/log.hpp>
#include <lemon/test/unit.hpp>
#include <lemon/test/macro.hpp>
#include <lemon/test/runner.hpp>



namespace lemon {namespace test{

    runner& runner::instance()
    {
        static runner global;

        return global;
    }

    void runner::run() {
        runner::instance().done();
    }

    void runner::done() {

        auto & logger = log::get("test");

        for(auto unit : _units)
        {
            try
            {
                unit->run();

                lemonI(logger,"test(%s) ... ok",unit->name().c_str());
            }
            catch(const assert & e)
            {
                lemonE(logger,"test(%s) -- failed\n\terr :%s\n\tfile :%s(%d)",unit->name().c_str(),e.what(),e.file().c_str(),e.lines());
            }
            catch(const std::exception &e)
            {
                lemonE(logger,"test(%s) -- failed\n\terr :%s\n\tfile :%s(%d)",unit->name().c_str(),e.what(),unit->file().c_str(),unit->lines());
            }
			catch(...)
			{
				lemonE(logger, "test(%s) -- failed\n\terr :unknown exception\n\tfile :%s(%d)", unit->name().c_str(), unit->file().c_str(), unit->lines());
			}

        }
    }

    void runner::add(runnable *unit)
    {
        _units.push_back(unit);
    }
}}