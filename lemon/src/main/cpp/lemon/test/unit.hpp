#ifndef LEMON_TEST_UNIT_HPP
#define LEMON_TEST_UNIT_HPP

#include <string>
#include <chrono>
#include <lemon/nocopy.hpp>

namespace lemon { namespace test {
    /**
    * The runnable interface defined the test/benchmark unit
    */
    class runnable : private nocopy
    {
    public:
        /**
         * runner call this function to invoke current test/bench unit
         */
        virtual void run() = 0;

        virtual const std::string & name() const = 0;

        virtual const std::string & file() const = 0;

        virtual int lines() const = 0;
    };


    /**
     * the test unit class hold the test function object and test name
     */
    class unit: public runnable
    {
    public:

        unit(const std::string & name, const std::string & filename, int lines);

        /**
         * get the test name;
         */
        const std::string & name() const final;

        /**
         * get test file path;
         */
        const std::string & file() const final;

        /**
         * get test start lines
         */
        int lines() const final;

        /**
         * test main method
         */
        virtual void main() = 0;

    private:

        std::string                     _name;
        std::string                     _filename;
        int                             _lines;
    };

    class T : public unit
    {
    public:
        T(const std::string & name, const std::string & filename, int lines);

        void run();
    };

    class B : public unit
    {
    public:

        B(const std::string & name, const std::string & filename, int lines);

        void run();

		void stop_timer();

		void start_timer();

    public:

        int             N;

	private:

		std::chrono::high_resolution_clock::time_point		_stop_timepoint;

		std::chrono::nanoseconds							_stopduration;
    };
}}

#endif //LEMON_TEST_UNIT_HPP