#ifndef LEMON_TEST_MACRO_HPP
#define LEMON_TEST_MACRO_HPP

#include <stdexcept>
#include <lemon/test/unit.hpp>
#include <lemon/test/runner.hpp>

namespace lemon { namespace  test{
    class assert : public std::runtime_error
    {
    public:
        assert(const char * msg, const char * file,size_t lines)
            :runtime_error(msg)
            ,_file(file)
            ,_lines(lines)
        {

        }

        const std::string& file() const { return _file; }


        size_t lines() const { return _lines; }

    private:
        std::string										_file;

        size_t											_lines;
    };

    inline void check(bool condition,const char * message,const char * file,int lines)
    {
        if(!condition)
        {
            throw assert(message,file,lines);
        }
    }
}}

#define test_(name)\
class test_##name : public lemon::test::T {\
public:\
    test_##name(const std::string & name, const std::string & f, int lines):T(name, f,lines){}\
    void main() final;\
};\
namespace{\
    test_##name test_##name##_instance(#name,__FILE__,__LINE__); \
}\
void test_##name::main()

#define bench_(name)\
class bench_##name : public lemon::test::B {\
public:\
    bench_##name(const std::string & name, const std::string & filename, int lines):B(name, filename,lines){}\
    void main() final;\
};\
namespace{\
    bench_##name bench_##name##_instance(#name,__FILE__,__LINE__); \
}\
void bench_##name::main()

#define test_assert(block) lemon::test::check((bool)(block),"condition check error --> "#block,__FILE__,__LINE__)

#endif //LEMON_TEST_MACRO_HPP