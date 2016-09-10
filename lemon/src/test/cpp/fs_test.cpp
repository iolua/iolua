#include <lemon/fs/fs.hpp>
#include <lemon/log/log.hpp>
#include <lemon/test/test.hpp>

using namespace lemon::fs;
using namespace lemon::log;

test_(filename){


    test_assert(filepath("/").filename().generic_string() == "/");
    test_assert(filepath("/.").filename().generic_string() == "/");
    test_assert(filepath("/").generic_string() == "/");

    test_assert(filepath("").filename().generic_string() == "");

    test_assert(filepath(".").filename().generic_string() == ".");

    test_assert(filepath("./").filename().generic_string() == ".");
    test_assert(filepath("./").generic_string() == "./");

    test_assert(filepath("./name").generic_string() == "./name");
    test_assert(filepath("./name").filename().generic_string() == "name");

    test_assert(filepath("./name/").generic_string() == "./name/");
    test_assert(filepath("./name/").filename().generic_string() == ".");

    test_assert(filepath("./name/..").generic_string() == "./name/..");
    test_assert(filepath("./name/..").filename().generic_string() == "..");

    test_assert(filepath("./name.txt").extension().generic_string() == ".txt");
    test_assert(filepath("./.txt").extension().generic_string() == ".txt");

    test_assert(filepath("./name.java.txt").stem().generic_string() == "name.java");

    test_assert(filepath("name").generic_string() == "name");

    test_assert(filepath("/name/").parent_path().generic_string() == "/name");

    test_assert(filepath("/name/.").parent_path().generic_string() == "/name");

    test_assert(filepath("/name").parent_path().generic_string() == "/");
}

test_(funcs){

    test_assert(exists("./"));

    if(exists("./test"))
    {
        remove_directories("./test");
    }

    create_directory("./test");

    test_assert(exists("./test"));

    create_symlink("./test","./test_symlink");

    if(exists("./test_N"))
    {
        remove_directories("./test_N");
    }

    create_directories("./test_N/test1/test2");

    test_assert(is_directory("./test"));

    test_assert(is_directory("./test_symlink"));

    test_assert(exists("./test_symlink"));

    lemonI(get("test"),"%s",absolute("./test_symlink").generic_string().c_str());

    remove_file("./test_symlink");

    remove_file("./test");

    remove_directories("./test_N");

}
